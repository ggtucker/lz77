#include "LZCompressor.h"
#include <fstream>
#include <cassert>
#include <algorithm>
#include <cstdlib>
#include <cmath>

namespace lz {

	Compressor::Compressor(int windowOffsetBits, int matchLengthBits, int literalLengthBits) :
		m_windowOffsetBits( windowOffsetBits ),
		m_matchLengthBits( matchLengthBits ),
		m_literalLengthBits( literalLengthBits ),
		m_literalQueue(),
		m_uncompressedBytes( 0 ),
		m_compressedBytes( 3 ),
		m_numProcessedBytes( 0 ),
		m_encodeTime( 0 ),
		m_shiftTime( 0 ),
		m_searchTime( 0 ),
		m_currentOffset( 0 ) {

		if (m_windowOffsetBits < MIN_WINDOW_OFFSET_BITS || m_windowOffsetBits > MAX_WINDOW_OFFSET_BITS) {
			std::cerr << "N was " << m_windowOffsetBits << ", but must be in range ["
				<< MIN_WINDOW_OFFSET_BITS << "," << MAX_WINDOW_OFFSET_BITS << "]";
			exit(EXIT_FAILURE);
		} else if (m_matchLengthBits < MIN_MATCH_LENGTH_BITS || m_matchLengthBits > MAX_MATCH_LENGTH_BITS) {
			std::cerr << "L was " << m_matchLengthBits << ", but must be in range ["
				<< MIN_MATCH_LENGTH_BITS << "," << MAX_MATCH_LENGTH_BITS << "]";
			exit(EXIT_FAILURE);
		} else if (m_literalLengthBits < MIN_LITERAL_LENGTH_BITS || m_literalLengthBits > MAX_LITERAL_LENGTH_BITS) {
			std::cerr << "S was " << m_literalLengthBits << ", but must be in range ["
				<< MIN_LITERAL_LENGTH_BITS << "," << MAX_LITERAL_LENGTH_BITS << "]";
			exit(EXIT_FAILURE);
		}

		m_windowSize = static_cast<int>(std::pow(2, m_windowOffsetBits));
		m_maxMatchLength = static_cast<int>(std::pow(2, m_matchLengthBits)) - 1;
		m_maxLiteralLength = static_cast<int>(std::pow(2, m_literalLengthBits)) - 1;
		m_bufferSize = m_maxMatchLength;
		m_processedSize = m_windowSize - m_bufferSize;
		m_bufferSize = m_maxMatchLength;
		m_processedSize = m_windowSize - m_bufferSize;
		m_window = CircularVector<Byte>(m_processedSize, 0);
		m_lookahead = CircularVector<Byte>(m_bufferSize, 0);
	}

	void Compressor::Compress(const std::string& fileName) {
		std::cerr << "=================================================" << std::endl;
		std::cerr << "|            COMPRESSION PARAMETERS             |" << std::endl;
		std::cerr << "=================================================" << std::endl;
		if (fileName.length() > 0) {
			std::ifstream fileStream(fileName, std::ios::in | std::ios::binary);
			if (fileStream) {
				std::cerr << "Compressing file " << fileName << std::endl;
				Compress(fileStream);
			}
		}
		else {
			std::cerr << "Compressing standard input" << std::endl;
			Compress();
		}
	}

	void Compressor::Compress(std::istream& in) {
		std::cerr << "[N] Window offset bits: " << m_windowOffsetBits << std::endl;
		std::cerr << "[L] Match length bits: " << m_matchLengthBits << std::endl;
		std::cerr << "[S] Literal length bits: " << m_literalLengthBits << std::endl;

		clock_t time = clock();

		Byte N = m_windowOffsetBits;
		Byte L = m_matchLengthBits;
		Byte S = m_literalLengthBits;
		std::cout << N << L << S;
		
		slide(in, m_bufferSize, false);

		Byte outputByte = 0;
		int usedBits = 0;

		while (m_bufferSize > 0) {

			int length, offset;
			findLongestMatch(length, offset);

			if (length >= 2) {
				// print <len,offset> pair
				printLengthOffset(outputByte, usedBits, length, offset);
			}
			else {
				// enqueue literal char, print <code,strlen,c1,c2,...> if queue reaches max literal length
				enqueueLiteral(outputByte, usedBits, m_lookahead[0]);
				length = 1;
			}

			slide(in, length, true);
		}

		printEOF(outputByte, usedBits);

		time = clock() - time;

		std::cerr << "=================================================" << std::endl;
		std::cerr << "|              COMPRESSION RESULTS              |" << std::endl;
		std::cerr << "=================================================" << std::endl;
		std::cerr << "Compression Savings: " << (100.0f * (float)m_compressedBytes / (float)m_uncompressedBytes) << "% original file size " << std::endl;
		std::cerr << "Compressed Bytes: " << m_compressedBytes << " bytes" << std::endl;
		std::cerr << "Uncompressed Bytes: " << m_uncompressedBytes << " bytes" << std::endl;
		std::cerr << "Processing Time: " << ((float)time / (float)CLOCKS_PER_SEC) << " seconds" << std::endl;
		std::cerr << "  Shift Time: " << ((float)m_shiftTime / (float)CLOCKS_PER_SEC) << " seconds" << std::endl;
		std::cerr << "  Search Time: " << ((float)m_searchTime / (float)CLOCKS_PER_SEC) << " seconds" << std::endl;
		std::cerr << "  Encode Time: " << ((float)m_encodeTime / (float)CLOCKS_PER_SEC) << " seconds" << std::endl;
	}

	void Compressor::printBits(Byte& outputByte, int& usedBits, int value, int numBits) {
		clock_t time = clock();

		int remainingBits = numBits;
		while (remainingBits > 0) {
			int bitsToAppend = std::min(8 - usedBits, remainingBits);
			int mask = (static_cast<int>(std::pow(2, remainingBits)) - 1) - (static_cast<int>(std::pow(2, remainingBits - bitsToAppend)) - 1);

			outputByte = outputByte << bitsToAppend;
			outputByte |= (mask & value) >> (remainingBits - bitsToAppend);
			usedBits = (usedBits + bitsToAppend) % 8;

			if (usedBits == 0) {
				std::cout << outputByte;
				outputByte = 0;
				++m_compressedBytes;
			}

			remainingBits -= bitsToAppend;
		}

		m_encodeTime += (clock() - time);
	}

	void Compressor::slideMatchMap() {

		if (m_currentOffset >= m_window.Size()) {
			CharPair toRemove = { m_window[0], m_window[1] };
			std::list<int>& offsets = m_matchMap[toRemove];
			auto it = std::find(offsets.begin(), offsets.end(), m_currentOffset - m_window.Size() + 1);
			if (it != offsets.end()) {
				offsets.erase(it);
			}
		}

		m_window[0] = m_lookahead[0];
		m_window.Shift(1);

		if (m_currentOffset > 0) {
			CharPair toAdd = { m_window[m_window.Size() - 2], m_window[m_window.Size() - 1] };
			std::list<int>& offsets = m_matchMap[toAdd];
			offsets.push_back(m_currentOffset);
		}

		++m_currentOffset;
	}

	void Compressor::slide(std::istream& in, int numBytes, bool shiftWindow) {
		clock_t time = clock();

		char nextChar;
		int shift = 0;
		while (shift < numBytes && in.get(nextChar)) {
			if (shiftWindow) {
				slideMatchMap();
			}
			m_lookahead[0] = nextChar;
			m_lookahead.Shift(1);
			++shift;
			++m_uncompressedBytes;
		}

		while (shift < numBytes && m_bufferSize > 0) {
			if (shiftWindow) {
				slideMatchMap();
			}
			m_lookahead.Shift(1);
			++shift;
			--m_bufferSize;
		}

		m_shiftTime += (clock() - time);
	}

	void Compressor::enqueueLiteral(Byte& outputByte, int& usedBits, Byte literalByte) {
		m_literalQueue.push(literalByte);
		if (m_literalQueue.size() == m_maxLiteralLength) {
			printLiteral(outputByte, usedBits);
		}
	}

	void Compressor::findLongestMatch(int& length, int& offset) {
		clock_t time = clock();

		length = 0;
		offset = 0;

		CharPair key = { m_lookahead[0], m_lookahead[1] };
		std::list<int>& offsets = m_matchMap[key];

		for (auto it = offsets.begin(); it != offsets.end(); ++it) {
			int start = (*it - 1) % m_window.Size();

			int curLength = 2;
			while (curLength < m_bufferSize && (m_window.Get(start + curLength) == m_lookahead[curLength])) {
				++curLength;
			}

			if (curLength > length) {
				length = curLength;
				int tempOffset = (m_window.ShiftIndex(0) - start);
				if (tempOffset < 0) {
					tempOffset += m_window.Size();
				}
				offset = tempOffset;
			}
		}

		m_searchTime += (clock() - time);
	}

	void Compressor::printLengthOffset(Byte& outputByte, int& usedBits, int length, int offset) {
		// flush the queued literal string
		printLiteral(outputByte, usedBits);

		printBits(outputByte, usedBits, length - 1, m_matchLengthBits);
		printBits(outputByte, usedBits, offset, m_windowOffsetBits);
	}

	void Compressor::printLiteral(Byte& outputByte, int& usedBits) {
		assert(m_literalQueue.size() <= m_maxLiteralLength);

		if (m_literalQueue.empty()) return;

		printBits(outputByte, usedBits, 0, m_matchLengthBits);
		printBits(outputByte, usedBits, m_literalQueue.size(), m_literalLengthBits);

		while (!m_literalQueue.empty()) {
			printBits(outputByte, usedBits, m_literalQueue.front(), 8);
			m_literalQueue.pop();
		}
	}

	void Compressor::printEOF(Byte& outputByte, int& usedBits) {
		// flush the queued literal string
		printLiteral(outputByte, usedBits);

		printBits(outputByte, usedBits, 0, m_matchLengthBits);
		printBits(outputByte, usedBits, 0, m_literalLengthBits);

		// Flush the output byte by printing remaining bits as 0
		printBits(outputByte, usedBits, 0, 8 - usedBits);
	}

}
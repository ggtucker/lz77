#include "LZExpander.h"
#include <fstream>
#include <cassert>
#include <algorithm>
#include <iomanip>

namespace lz {

	Expander::Expander() {

	}

	void Expander::Expand(const std::string& fileName) {
		std::cerr << "=================================================" << std::endl;
		std::cerr << "|           DECOMPRESSION PARAMETERS            |" << std::endl;
		std::cerr << "=================================================" << std::endl;
		if (fileName.length() > 0) {
			std::ifstream fileStream(fileName, std::ios::in | std::ios::binary);
			if (fileStream) {
				std::cerr << "Expanding file " << fileName << std::endl;
				Expand(fileStream);
			}
		}
		else {
			std::cerr << "Expanding standard input" << std::endl;
			Expand();
		}
	}

	void Expander::Expand(std::istream& in) {
		std::clock_t time = std::clock();

		initialize(in);
		std::cerr << "[N] Window offset bits: " << m_windowOffsetBits << std::endl;
		std::cerr << "[L] Match length bits: " << m_matchLengthBits << std::endl;
		std::cerr << "[S] Literal length bits: " << m_literalLengthBits << std::endl;

		CircularVector<Byte> window(m_processedSize);

		Byte inputByte = 0;
		int usedBits = 0;

		while (true) {
			int length = 1 + readBits(inputByte, usedBits, in, m_matchLengthBits);

			if (length >= 2) {
				readMatch(inputByte, usedBits, window, in, length);
			}
			else {
				if (!readLiteral(inputByte, usedBits, window, in)) break;
			}
		}

		time = std::clock() - time;

		std::cerr << std::fixed << std::setprecision(2) << std::endl;
		std::cerr << "=================================================" << std::endl;
		std::cerr << "|             DECOMPRESSION RESULTS             |" << std::endl;
		std::cerr << "=================================================" << std::endl;
		std::cerr << "Processing Time: " << ((float)time / (float)CLOCKS_PER_SEC) << " seconds" << std::endl;
	}

	int Expander::readBits(Byte& inputByte, int& usedBits, std::istream& in, int numBits) {
		int value = 0;
		while (numBits > 0) {
			if (usedBits == 0) {
				inputByte = in.get();
			}
			
			int bitsToRead = std::min(8 - usedBits, numBits);

			int mask = 255 - (static_cast<int>(pow(2, 8 - bitsToRead)) - 1);
			value = value << bitsToRead;
			value |= (mask & (inputByte << usedBits)) >> (8 - bitsToRead);

			usedBits = (usedBits + bitsToRead) % 8;

			numBits -= bitsToRead;
		}
		return value;
	}

	void Expander::shiftWindow(CircularVector<Byte>& window, const std::vector<Byte>& bytes) {

		for (int i = 0; i < bytes.size(); ++i) {
			window[0] = bytes[i];
			window.Shift(1);
		}

	}

	void Expander::initialize(std::istream& in) {
		m_windowOffsetBits = in.get();
		m_matchLengthBits = in.get();
		m_literalLengthBits = in.get();
		m_windowSize = static_cast<int>(pow(2, m_windowOffsetBits));
		m_maxMatchLength = static_cast<int>(pow(2, m_matchLengthBits)) - 1;
		m_maxLiteralLength = static_cast<int>(pow(2, m_literalLengthBits)) - 1;
		m_bufferSize = m_maxMatchLength;
		m_processedSize = m_windowSize - m_bufferSize;

		if (m_windowOffsetBits < MIN_WINDOW_OFFSET_BITS || m_windowOffsetBits > MAX_WINDOW_OFFSET_BITS) {
			throw std::invalid_argument("N was " + std::to_string(m_windowOffsetBits) + ", but must be in range ["
				+ std::to_string(MIN_WINDOW_OFFSET_BITS) + "," + std::to_string(MAX_WINDOW_OFFSET_BITS) + "]");
		} else if (m_matchLengthBits < MIN_MATCH_LENGTH_BITS || m_matchLengthBits > MAX_MATCH_LENGTH_BITS) {
			throw std::invalid_argument("L was " + std::to_string(m_matchLengthBits) + ", but must be in range ["
				+ std::to_string(MIN_MATCH_LENGTH_BITS) + "," + std::to_string(MAX_MATCH_LENGTH_BITS) + "]");
		} else if (m_literalLengthBits < MIN_LITERAL_LENGTH_BITS || m_literalLengthBits > MAX_LITERAL_LENGTH_BITS) {
			throw std::invalid_argument("S was " + std::to_string(m_literalLengthBits) + ", but must be in range ["
				+ std::to_string(MIN_LITERAL_LENGTH_BITS) + "," + std::to_string(MAX_LITERAL_LENGTH_BITS) + "]");
		}
	}

	void Expander::readMatch(Byte& inputByte, int& usedBits, CircularVector<Byte>& window, std::istream& in, int length) {
		int offset = readBits(inputByte, usedBits, in, m_windowOffsetBits);

		std::vector<Byte> bytes;

		for (int i = 0; i < length; ++i) {
			Byte value = window[(window.Size() - offset + i) % window.Size()];
			bytes.push_back(value);
			std::cout << value;
		}

		shiftWindow(window, bytes);
	}

	bool Expander::readLiteral(Byte& inputByte, int& usedBits, CircularVector<Byte>& window, std::istream& in) {
		int strlen = readBits(inputByte, usedBits, in, m_literalLengthBits);

		if (strlen == 0) return false;

		std::vector<Byte> bytes;

		while (strlen > 0) {
			Byte value = readBits(inputByte, usedBits, in, 8);
			bytes.push_back(value);
			std::cout << value;
			--strlen;
		}

		shiftWindow(window, bytes);

		return true;
	}

}
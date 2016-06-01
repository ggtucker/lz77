#pragma once

#include "LZTypedefs.h"
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <list>
#include <ctime>
#include "CircularVector.h"

namespace lz {

	class Compressor {
	private:
		// Parameterized members
		int m_windowOffsetBits;  // Range: [9, 14]
		int m_matchLengthBits;   // Range: [3,  4]
		int m_literalLengthBits; // Range: [1,  5]

		// Derived members
		int m_windowSize;
		int m_maxMatchLength;
		int m_maxLiteralLength;
		int m_bufferSize;
		int m_processedSize;

		// Timers
		clock_t m_encodeTime;
		clock_t m_shiftTime;
		clock_t m_searchTime;

		// Implementation members
		typedef std::pair<char, char> CharPair;
		std::map<CharPair, std::list<int>> m_matchMap;
		CircularVector<Byte> m_window;
		CircularVector<Byte> m_lookahead;
		std::queue<Byte> m_literalQueue;
		unsigned int m_uncompressedBytes;
		unsigned int m_compressedBytes;
		unsigned int m_numProcessedBytes;
		int m_currentOffset;

	public:
		Compressor(int windowOffsetBits, int matchLengthBits, int literalLengthBits);

		void Compress(const std::string& fileName);
		void Compress(std::istream& in = std::cin);

	private:
		void printBits(Byte& outputByte, int& usedBits, int value, int numBits);

		void slideMatchMap();
		void slide(std::istream& in, int numBytes, bool shiftWindow);
		void enqueueLiteral(Byte& outputByte, int& usedBits, Byte literalByte);
		void findLongestMatch(int& length, int& offset);
		void printLengthOffset(Byte& outputByte, int& usedBits, int length, int offset);
		void printLiteral(Byte& outputByte, int& usedBits);
		void printEOF(Byte& outputByte, int& usedBits);
	};

}
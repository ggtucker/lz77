#pragma once

#include "LZTypedefs.h"
#include <iostream>
#include <string>
#include "CircularVector.h"

namespace lz {

	class Expander {
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

	public:
		Expander();

		void Expand(const std::string& fileName);
		void Expand(std::istream& in = std::cin);

	private:
		int readBits(Byte& inputByte, int& usedBits, std::istream& in, int numBits);

		void shiftWindow(CircularVector<Byte>& window, const std::vector<Byte>& bytes);
		void initialize(std::istream& in);
		void readMatch(Byte& inputByte, int& usedBits, CircularVector<Byte>& window, std::istream& in, int length);
		bool readLiteral(Byte& inputByte, int& usedBits, CircularVector<Byte>& window, std::istream& in);
	};

}
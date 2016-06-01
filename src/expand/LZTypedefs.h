#pragma once

namespace lz {

	static const int MIN_WINDOW_OFFSET_BITS = 9;
	static const int MAX_WINDOW_OFFSET_BITS = 14;
	static const int MIN_MATCH_LENGTH_BITS = 3;
	static const int MAX_MATCH_LENGTH_BITS = 4;
	static const int MIN_LITERAL_LENGTH_BITS = 1;
	static const int MAX_LITERAL_LENGTH_BITS = 5;

	using Byte = char;

}
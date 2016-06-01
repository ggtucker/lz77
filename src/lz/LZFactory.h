#pragma once

#include "LZCompressor.h"
#include <string>
#include <vector>

namespace lz {

	Compressor CreateCompressor(const std::vector<std::string>& args, std::string& fileName);

}
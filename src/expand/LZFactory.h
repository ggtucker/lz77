#pragma once

#include "LZExpander.h"
#include <string>
#include <vector>

namespace lz {

	Expander CreateExpander(const std::vector<std::string>& args, std::string& fileName);

}
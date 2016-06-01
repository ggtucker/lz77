#include "LZFactory.h"

namespace lz {

	Expander CreateExpander(const std::vector<std::string>& args, std::string& fileName) {

		for (const std::string& arg : args) {
			if (arg.length() > 0 && arg.substr(0, 1) != "-") {
				fileName = arg;
			}
		}

		return Expander();
	}

}
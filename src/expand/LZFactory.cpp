#include "LZFactory.h"

namespace lz {

	Expander CreateExpander(const std::vector<std::string>& args, std::string& fileName) {

		for (int i = 0; i < args.size(); ++i) {
			const std::string& arg = args[i];

			if (arg.length() > 0 && arg.substr(0, 1) != "-") {
				fileName = arg;
			}
		}

		return Expander();
	}

}
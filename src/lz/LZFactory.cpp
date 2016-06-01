#include "LZFactory.h"

namespace lz {

	Compressor CreateCompressor(const std::vector<std::string>& args, std::string& fileName) {

		int windowOffsetBits = 11;
		int matchLengthBits = 4;
		int literalLengthBits = 3;

		for (int i = 0; i < args.size(); ++i) {
			const std::string& arg = args[i];
			
			if (arg.length() > 0 && arg.substr(0, 1) != "-") {
				fileName = arg;
			}
			else if (arg.length() > 3) {
				if(arg.substr(0, 3) == "-N=") {
					windowOffsetBits = std::stoi(arg.substr(3));
				}
				else if (arg.substr(0, 3) == "-L=") {
					matchLengthBits = std::stoi(arg.substr(3));
				}
				else if (arg.substr(0, 3) == "-S=") {
					literalLengthBits = std::stoi(arg.substr(3));
				}
			}

		}

		return Compressor(windowOffsetBits, matchLengthBits, literalLengthBits);
	}

}
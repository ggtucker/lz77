#include "LZCompressor.h"
#include "LZFactory.h"
#include <fstream>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

std::vector<std::string> ParseArguments(int argc, char* argv[]) {
	std::vector<std::string> args;
	for (int i = 1; i < argc; ++i) {
		args.push_back(argv[i]);
	}
	return args;
}

void Compress(const std::vector<std::string>& args) {
	try {
		std::string fileName;
		lz::Compressor compressor = lz::CreateCompressor(args, fileName);
		compressor.Compress(fileName);
	}
	catch (const std::exception& e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
	}
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
	_setmode(_fileno(stdout), O_BINARY);
#endif

	Compress(ParseArguments(argc, argv));
}
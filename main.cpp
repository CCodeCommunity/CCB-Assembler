#include <iostream>
#include "assembler.h"

#include "./lib/cxxopt.hpp"
#include "./lib/termcolor.hpp"

int main(int argc, char* argv[]) {
	cxxopts::Options options("cca", "The official CC Assembler\n");

	options.add_options()
		("d,debug", "Prints the token array for debug")
		("s,silent", "Dont display any info except errors")
		("h,help", "Display this information")
		("v,version", "Display the assembler version")
		("w,watch", "Watch for file changes")
		("o,output", "Outputs the bytecode to the file named <arg>", cxxopts::value<std::string>());

	cxxopts::ParseResult result;
	
	try {
		result = options.parse(argc, argv);
	} catch (const cxxopts::OptionParseException& e) {
		std::cout << termcolor::red << "[ERROR] " << termcolor::reset << e.what() << "\n\n";
		std::exit(-1);
	}

	cxxopts::PositionalList args = result.unmatched();

	if (result.count("version")) {
		std::cout << "CCAssembler V1.0.0\n";
		std::exit(0);
	}

	if (result.count("help") || args.size() == 0) {
		std::cout << options.help() << "\n";
		std::exit(0);
	}

	if (args.size() > 0) {
		std::string fileName = args[0];

		if (result.count("watch"))
			CCA::watchAssembly(fileName, result);
		else
			CCA::assemble(fileName, result);

		std::exit(0);
	}
}

#include <iostream>
#include "assembler.h"

int main(int argc, char* argv[]) {
	if (argc >= 2) {
		std::string fileName = argv[1];
		CCA::assemble(fileName);
		return 0;
	} else {
		std::cout << "Usage 'cca filename.cca'" << "\n";
		return -1;
	}
}

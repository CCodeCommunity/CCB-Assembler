// stdlib headers
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <streambuf>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>

// other libraries
#include "./lib/termcolor.hpp"
#include "./lib/cxxopt.hpp"

// how to compile:
// g++ main.cpp -o cca -std=c++11 && ./cca test.cca

namespace CCA {
	bool in_array(const std::string &value, const std::vector<std::string> &array) {
		return std::find(array.begin(), array.end(), value) != array.end();
	}

	enum class TokenType {
		IDENTIFIER,
		NUMBER,
		DIVIDER,
		OPCODE,
		REGISTER,
		MARKER,
		END,
		ADDRESS,
		STRING,
		UNKNOWN
	};

	struct Token {
		TokenType type;
		int lineFound;
		std::string valString;
		int valNumeric;
		int byteIndex;
	};

	struct Definition {
		int index;
		std::string value;
		std::string name;
	};

	struct Marker {
		std::string name;
		int byteIndex;
	};
	
	std::string readFile(std::string &fileName) {
		std::ifstream file(fileName);
		std::string content;

		if (!file.is_open()) {
			std::cout << termcolor::red << "[ERROR]" << termcolor::reset << " Could not open file '" << fileName << "', are you sure it exists?\n\n";
			std::exit(-1);
		}

		file.seekg(0, std::ios::end);
		content.reserve(file.tellg());
		file.seekg(0, std::ios::beg);

		content.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		file.close();	
		return content;
	}

	bool isIgnorable(char c) {
		return c == ' ' || c == '\t' || c == '\n' || c == '\r';
	}

	bool isNumber(char c) {
		return c <= '9' && c >= '0';
	}

	bool isDivider(char c) {
		return c == ',';
	}

	bool isIdentifier(char c) {
		return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
	}

	bool isAddress(char c) {
		return c == '&';
	}

	bool isComment(char c) {
		return c == ';';
	}

	bool isString(char c) {
		return c == '\'' || c == '"';
	}

	bool isMarker(char c) {
		return c == ':';
	}

	std::string parseWord(std::string &code, unsigned int &readingIndex) {
		std::string result = "";
		result += code[readingIndex++];

		while(isIdentifier(code[readingIndex])) {
			result += code[readingIndex++];
		}

		--readingIndex;

		return result;
	}

	std::string parseString(std::string &code, unsigned int &readingIndex) {
		std::string result = "";
		result += code[readingIndex++];

		while(!isString(code[readingIndex])) {
			result += code[readingIndex++];
		}

		return result;
	}

	int parseNumber(std::string &code, unsigned int &readingIndex) {
		std::string result = "";
		result += code[readingIndex++];

		while(isNumber(code[readingIndex])) {
			result += code[readingIndex++];
		}

		--readingIndex;

		return std::stoi(result);
	}

	std::vector<Token> lexer(std::string code) {
		std::vector<Token> tokens;
		int lineFound = 1;
		bool error = false;
		bool foundDef = false;
		int byteIndex = 0;

		for (unsigned int readingIndex = 0; readingIndex < code.size(); readingIndex++) {
			char currentCharacter = code[readingIndex];

			if (currentCharacter == '\n') {
				lineFound++;
			}

			if (isIgnorable(currentCharacter)) {
				continue;
			}
			
			else if (isMarker(currentCharacter)) {
				++readingIndex;
				std::string value = parseWord(code, readingIndex);

				tokens.push_back(Token {
					TokenType::MARKER,
					lineFound,
					value,
					0,
					byteIndex
				});
			}
			
			else if (isDivider(currentCharacter)) {
				tokens.push_back(Token {
					TokenType::DIVIDER,
					lineFound,
					",",
					0,
					byteIndex
				});
			}
			
			else if (isIdentifier(currentCharacter)) {
				std::string value = parseWord(code, readingIndex);

				tokens.push_back(Token {
					TokenType::IDENTIFIER,
					lineFound,
					value,
					0,
					byteIndex
				});

				++byteIndex;

				if (foundDef) {
					foundDef = false;
					--byteIndex;
				} else if (value == "def") {
					foundDef = true;
					--byteIndex;
				} else if (value != "a" && value != "b" && value != "c" && value != "d"){
					byteIndex += 3;
				}
			}
			
			else if (isNumber(currentCharacter)) {
				int value = parseNumber(code, readingIndex);

				tokens.push_back(Token {
					TokenType::NUMBER,
					lineFound,
					"",
					value,
					byteIndex
				});

				byteIndex += 4;
			}
			
			else if (isAddress(currentCharacter)) {
				++readingIndex;
				int value = parseNumber(code, readingIndex);

				tokens.push_back(Token {
					TokenType::ADDRESS,
					lineFound,
					"",
					value,
					byteIndex
				});

				byteIndex += 4;
			}
			
			else if (isString(currentCharacter)) {
				++readingIndex;
				std::string value = parseString(code, readingIndex);

				tokens.push_back(Token {
					TokenType::STRING,
					lineFound,
					value,
					0,
					byteIndex
				});
			}

			else if (isComment(currentCharacter)) {
				++readingIndex;
				++lineFound;

				while (readingIndex <= code.size() && code[readingIndex] != '\n') {
					++readingIndex;
				}
			}
			
			else {
				std::cout << termcolor::red << "[ERROR]" << termcolor::reset << " Unexpected symbol on" << termcolor::red << " line " << lineFound << termcolor::reset;
				error = true;
			}
		}

		if (error) {
			std::cout << termcolor::red << "[ERROR]" << termcolor::reset << " Aborting due to errors while parsing\n";
			std::exit(-1);
		}

		return tokens;
	}

	std::string stringifyToken(TokenType value) {
		switch(value) {
			case TokenType::IDENTIFIER:
				return "identifier";
			case TokenType::NUMBER:
				return "number";
			case TokenType::DIVIDER:
				return "divider";
			case TokenType::OPCODE:
				return "opcode";
			case TokenType::REGISTER:
				return "register";
			case TokenType::MARKER:
				return "marker";
			case TokenType::END:
				return "end";
			case TokenType::ADDRESS:
				return "address";
			case TokenType::STRING:
				return "string";
			default:
				return "unknown";
		}
	}
	
	std::string stringifyTokenValue(Token t) {
		if (t.type == TokenType::ADDRESS || t.type == TokenType::NUMBER)
			return std::to_string(t.valNumeric);
		else 
			return t.valString;
	}

	void printTokens(std::vector<Token> &tokens) {
		for (auto &t: tokens) {
			if (t.type == TokenType::ADDRESS || t.type == TokenType::NUMBER) {
				std::cout
					<< "{\n\ttype: " << stringifyToken(t.type)
					<< ",\n\tvalue: " << t.valNumeric
					<< ",\n\tline: " << t.lineFound
					<< "\n},\n";
			} else {
				std::cout
					<< "{\n\ttype: " << stringifyToken(t.type)
					<< ",\n\tvalue: '" << t.valString
					<< "',\n\tline: " << t.lineFound
					<< "\n},\n";
			}
		}
	}

	std::vector<Definition> parseDefinitions(std::vector<Token> &tokens) {
		std::vector<Token> tempTokens;
		int definitionMemoryIndex = 0;
		std::vector<Definition> definitions;
		
		for (unsigned int i = 0; i < tokens.size(); i++) {
			Token t = tokens[i];
			
			if (t.type == TokenType::IDENTIFIER && t.valString == "def") {
				if (tokens[i + 1].type != TokenType::IDENTIFIER || tokens[i + 2].type != TokenType::STRING) {
					std::cout << termcolor::red << "[ERROR]" << termcolor::reset << " Unknown syntax in definition statement on " << termcolor::red << " line " << t.lineFound << termcolor::reset;
					std::exit(-1);
				}
				
				definitions.push_back(Definition {
					definitionMemoryIndex,
					tokens[i + 2].valString,
					tokens[i + 1].valString
				});

				i += 2;
				continue;
			} else {
				tempTokens.push_back(t);
			}
		}

		tokens = tempTokens;

		return definitions;
	}

	void postTokenizer(std::vector<Token> &tokens, std::vector<Marker> &markers) {
		std::vector<Token> partialCopy = {};
		
		for (unsigned int i = 0; i < tokens.size(); i++) {
			Token &t = tokens[i];

			std::vector<std::string> opcodes = { "mov", "stp", "syscall", "push", "pop", "dup", "add", "sub", "mul", "div", "not", "and", "or", "xor", "jmp", "je", "jne", "jg", "js", "jo", "frs", "inc", "dec", "call", "ret", "cmp" };
			std::vector<std::string> registers = { "a", "b", "c", "d" };

			// indentify the opcodes
			if (t.type == TokenType::IDENTIFIER && in_array(t.valString, opcodes))
				t.type = TokenType::OPCODE;

			// identify the registers
			if (t.type == TokenType::IDENTIFIER && in_array(t.valString, registers))
				t.type = TokenType::REGISTER;

			// markers
			if (t.type == TokenType::MARKER) {
				markers.push_back(Marker {
					t.valString,
					t.byteIndex
				});

			} else {
				partialCopy.push_back(t);
			}
		}
		
		bool errors = false;

		tokens = partialCopy;

		for (unsigned int i = 0; i < tokens.size(); i++) {
			Token &t = tokens[i];
			if (t.type == TokenType::IDENTIFIER) {
				t.type = TokenType::NUMBER;

				bool found;
				
				for (unsigned int j = 0; j < markers.size(); j++) {
					Marker &m = markers[j];
					if (m.name == t.valString) {
						t.valNumeric = m.byteIndex;
						found = true;
						break;
					}
				}

				if (!found) {
					std::cout << termcolor::red << "[ERROR]" << termcolor::reset << " Could not match identifier '" << t.valString << "' on" << termcolor::red << " line " << t.lineFound << termcolor::reset << "\n\n";
					errors = true;
				}

				tokens[i] = t;
			}
		}
		
		if (errors) {
			std::cout << termcolor::red << "[ERROR]" << termcolor::reset << " Aborting due to errors while analyzing semantics\n\n";
			std::exit(-1);
		}

		
	}

	void pushRegister(std::vector<unsigned char> &bytecode, const Token &t) {
		bytecode.push_back(t.valString[0] - 'a');
	}

	void pushNumeric(std::vector<unsigned char> &bytecode, const Token &t) {
		for (int i = 0; i < 4; i++) {
			unsigned char byte = (t.valNumeric >> (24 - 8 * i)) & 0xFF;
			bytecode.push_back(byte);
		}
	}

	void pushLabel(std::vector<unsigned char> &bytecode, const Token &t) {
		for (int i = 0; i < 4; i++) {
			unsigned char byte = (t.byteIndex >> (24 - 8 * i)) & 0xFF;
			bytecode.push_back(byte);
		}
	}

	void generateBytecode(std::vector<Definition> definitions, std::vector<Token> tokens, std::string fileName) {
		std::vector<unsigned char> bytecode;

		bool error = false;
		
		for (unsigned int i = 0; i < tokens.size(); i++) {
			const Token &opcode = tokens[i];

			if (opcode.type != TokenType::OPCODE) {
				std::cout << termcolor::red << "[ERROR]" << termcolor::reset << " Expected opcode on line " << opcode.lineFound << " got " << stringifyToken(opcode.type) << ": " << stringifyTokenValue(opcode) << "\n";
				std::exit(-1);
			} else if (opcode.valString == "stp") {
				bytecode.push_back(0x00);
			} else if (opcode.valString == "syscall") {
				bytecode.push_back(0xff);
			} else if (opcode.valString == "push") {
				if (tokens[i + 1].type == TokenType::NUMBER) {
					bytecode.push_back(0x01);
					pushNumeric(bytecode, tokens[i + 1]);
				} else if (tokens[i + 1].type == TokenType::REGISTER) {
					bytecode.push_back(0x02);
					pushRegister(bytecode, tokens[i + 1]);
				} else if (tokens[i + 1].type == TokenType::ADDRESS) {	
					bytecode.push_back(0x0c);
					pushNumeric(bytecode, tokens[i + 1]);
				} else {
					std::cout << termcolor::red << "[ERROR]" << termcolor::reset << " Unknown structure for 'psh' mnemonic on line " << opcode.lineFound
						<< ".\nExpected one of the following: \n"
						<< "  - push <number>\n"
						<< "  - push <register>\n"
						<< "  - push <address>\n"
						<< "\n";

					error = true;
				}

				i += 1;
			} else if (opcode.valString == "pop") {
				if (tokens[i + 1].type == TokenType::REGISTER) {
					bytecode.push_back(0x03);
					pushRegister(bytecode, tokens[i + 1]);
				} else if (tokens[i + 1].type == TokenType::ADDRESS) {	
					bytecode.push_back(0x04);
					pushNumeric(bytecode, tokens[i + 1]);
				} else {
					std::cout << termcolor::red << "[ERROR]" << termcolor::reset << " Unknown structure for 'pop' mnemonic on line " << opcode.lineFound
						<< ".\nExpected one of the following: \n"
						<< "  - pop <register>\n"
						<< "  - pop <address>\n"
						<< "\n";

					error = true;
				}

				i += 1;
			} else if (opcode.valString == "dup") {
				bytecode.push_back(0x05);
			} else if (opcode.valString == "mov") {
				if (tokens[i + 1].type == TokenType::REGISTER && tokens[i + 2].type == TokenType::DIVIDER && tokens[i + 3].type == TokenType::NUMBER) {
					bytecode.push_back(0x06);
					pushRegister(bytecode, tokens[i + 1]);
					pushNumeric(bytecode, tokens[i + 3]);
				} else if (tokens[i + 1].type == TokenType::ADDRESS && tokens[i + 2].type == TokenType::DIVIDER && tokens[i + 3].type == TokenType::NUMBER) {
					bytecode.push_back(0x07);
					pushNumeric(bytecode, tokens[i + 1]);
					pushNumeric(bytecode, tokens[i + 3]);
				} else if (tokens[i + 1].type == TokenType::REGISTER && tokens[i + 2].type == TokenType::DIVIDER && tokens[i + 3].type == TokenType::ADDRESS) {
					bytecode.push_back(0x08);
					pushRegister(bytecode, tokens[i + 1]);
					pushNumeric(bytecode, tokens[i + 3]);
				} else if (tokens[i + 1].type == TokenType::ADDRESS && tokens[i + 2].type == TokenType::DIVIDER && tokens[i + 3].type == TokenType::REGISTER) {
					bytecode.push_back(0x09);
					pushNumeric(bytecode, tokens[i + 1]);
					pushRegister(bytecode, tokens[i + 3]);
				} else if (tokens[i + 1].type == TokenType::REGISTER && tokens[i + 2].type == TokenType::DIVIDER && tokens[i + 3].type == TokenType::REGISTER) {
					bytecode.push_back(0x0a);
					pushRegister(bytecode, tokens[i + 1]);
					pushRegister(bytecode, tokens[i + 3]);
				} else if (tokens[i + 1].type == TokenType::ADDRESS && tokens[i + 2].type == TokenType::DIVIDER && tokens[i + 3].type == TokenType::ADDRESS) {
					bytecode.push_back(0x0b);
					pushNumeric(bytecode, tokens[i + 1]);
					pushNumeric(bytecode, tokens[i + 3]);
				} else {
					std::cout << termcolor::red << "[ERROR]" << termcolor::reset << " Unknown structure for 'mov' mnemonic on" << termcolor::red << " line " << opcode.lineFound << termcolor::reset
						<< ".\nExpected one of the following: \n"
						<< "  - mov <register>, <number>\n"
						<< "  - mov <address>, <number>\n"
						<< "  - mov <register>, <address>\n"
						<< "  - mov <address>, <register>\n"
						<< "  - mov <register>, <register>\n"
						<< "  - mov <address>, <address>\n"
						<< "\n";

					error = true;
				}

				i += 3;
			} else if (opcode.valString == "add") {
				if (tokens[i + 1].type == TokenType::REGISTER && tokens[i + 2].type == TokenType::DIVIDER && tokens[i + 3].type == TokenType::REGISTER) {
					bytecode.push_back(0x10);
					pushRegister(bytecode, tokens[i + 1]);
					pushRegister(bytecode, tokens[i + 3]);
					i += 3;
				} else {
					bytecode.push_back(0x11);
				}
			} else if (opcode.valString == "sub") {
				if (tokens[i + 1].type == TokenType::REGISTER && tokens[i + 2].type == TokenType::DIVIDER && tokens[i + 3].type == TokenType::REGISTER) {
					bytecode.push_back(0x12);
					pushRegister(bytecode, tokens[i + 1]);
					pushRegister(bytecode, tokens[i + 3]);
					i += 3;
				} else {
					bytecode.push_back(0x13);
				}
			} else if (opcode.valString == "mul") {
				if (tokens[i + 1].type == TokenType::REGISTER && tokens[i + 2].type == TokenType::DIVIDER && tokens[i + 3].type == TokenType::REGISTER) {
					bytecode.push_back(0x14);
					pushRegister(bytecode, tokens[i + 1]);
					pushRegister(bytecode, tokens[i + 3]);
					i += 3;
				} else {
					bytecode.push_back(0x15);
				}
			} else if (opcode.valString == "div") {
				if (tokens[i + 1].type == TokenType::REGISTER && tokens[i + 2].type == TokenType::DIVIDER && tokens[i + 3].type == TokenType::REGISTER) {
					bytecode.push_back(0x16);
					pushRegister(bytecode, tokens[i + 1]);
					pushRegister(bytecode, tokens[i + 3]);
					i += 3;
				} else {
					bytecode.push_back(0x17);
				}
			} else if (opcode.valString == "not") {
				if (tokens[i + 1].type == TokenType::REGISTER) {
					bytecode.push_back(0x18);
					pushRegister(bytecode, tokens[i + 1]);
					i += 3;
				} else {
					bytecode.push_back(0x19);
				}
			} else if (opcode.valString == "and") {
				if (tokens[i + 1].type == TokenType::REGISTER && tokens[i + 2].type == TokenType::DIVIDER && tokens[i + 3].type == TokenType::REGISTER) {
					bytecode.push_back(0x1a);
					pushRegister(bytecode, tokens[i + 1]);
					pushRegister(bytecode, tokens[i + 3]);
					i += 3;
				} else {
					bytecode.push_back(0x1b);
				}
			} else if (opcode.valString == "or") {
				if (tokens[i + 1].type == TokenType::REGISTER && tokens[i + 2].type == TokenType::DIVIDER && tokens[i + 3].type == TokenType::REGISTER) {
					bytecode.push_back(0x1c);
					pushRegister(bytecode, tokens[i + 1]);
					pushRegister(bytecode, tokens[i + 3]);
					i += 3;
				} else {
					bytecode.push_back(0x1d);
				}
			} else if (opcode.valString == "xor") {
				if (tokens[i + 1].type == TokenType::REGISTER && tokens[i + 2].type == TokenType::DIVIDER && tokens[i + 3].type == TokenType::REGISTER) {
					bytecode.push_back(0x1e);
					pushRegister(bytecode, tokens[i + 1]);
					pushRegister(bytecode, tokens[i + 3]);
					i += 3;
				} else {
					bytecode.push_back(0x1f);
				}
			} else if (opcode.valString == "jmp") {
				if (tokens[i + 1].type == TokenType::NUMBER) {
					bytecode.push_back(0x20);
					pushNumeric(bytecode, tokens[i + 1]);
					i += 1;
				} else {
					std::cout << termcolor::red << "[ERROR]" << termcolor::reset <<  " Unknown structure for 'jmp' mnemonic on line " << opcode.lineFound
						<< ".\nExpected one of the following: \n"
						<< "  - jmp <number>\n"
						<< "\n";

					error = true;
				}
			} else if (opcode.valString == "cmp") {
				if (tokens[i + 1].type == TokenType::REGISTER && tokens[i + 2].type == TokenType::DIVIDER && tokens[i + 3].type == TokenType::REGISTER) {
					bytecode.push_back(0x30);
					pushRegister(bytecode, tokens[i + 1]);
					pushRegister(bytecode, tokens[i + 3]);
					i += 3;
				} else if (tokens[i + 1].type == TokenType::REGISTER && tokens[i + 2].type == TokenType::DIVIDER && tokens[i + 3].type == TokenType::NUMBER) {
					bytecode.push_back(0x31);
					pushRegister(bytecode, tokens[i + 1]);
					pushNumeric(bytecode, tokens[i + 3]);
					i += 3;
				} else if (tokens[i + 1].type == TokenType::NUMBER) {
					bytecode.push_back(0x32);
					pushNumeric(bytecode, tokens[i + 1]);
					i += 1;
				} else {
					std::cout << termcolor::red << "[ERROR]" << termcolor::reset <<  " Unknown structure for 'cmp' mnemonic on line " << opcode.lineFound
						<< ".\nExpected one of the following: \n"
						<< "  - cmp <register>, <register>\n"
						<< "  - cmp <register>, <number>\n"
						<< "  - cmp <number>\n"
						<< "\n";

					error = true;
				}
			} else if (opcode.valString == "je") {
				if (tokens[i + 1].type == TokenType::NUMBER) {
					bytecode.push_back(0x33);
					pushNumeric(bytecode, tokens[i + 1]);
					i += 1;
				} else {
					std::cout << termcolor::red << "[ERROR]" << termcolor::reset <<  " Unknown structure for 'je' mnemonic on line " << opcode.lineFound
						<< ".\nExpected one of the following: \n"
						<< "  - je <number>\n"
						<< "\n";

					error = true;
				}
			} else if (opcode.valString == "jne") {
				if (tokens[i + 1].type == TokenType::NUMBER) {
					bytecode.push_back(0x34);
					pushNumeric(bytecode, tokens[i + 1]);
					i += 1;
				} else {
					std::cout << termcolor::red << "[ERROR]" << termcolor::reset <<  " Unknown structure for 'jne' mnemonic on line " << opcode.lineFound
						<< ".\nExpected one of the following: \n"
						<< "  - jne <number>\n"
						<< "\n";

					error = true;
				}
			} else if (opcode.valString == "jg") {
				if (tokens[i + 1].type == TokenType::NUMBER) {
					bytecode.push_back(0x35);
					pushNumeric(bytecode, tokens[i + 1]);
					i += 1;
				} else {
					std::cout << termcolor::red << "[ERROR]" << termcolor::reset <<  " Unknown structure for 'jg' mnemonic on line " << opcode.lineFound
						<< ".\nExpected one of the following: \n"
						<< "  - jg <number>\n"
						<< "\n";

					error = true;
				}
			} else if (opcode.valString == "js") {
				if (tokens[i + 1].type == TokenType::NUMBER) {
					bytecode.push_back(0x36);
					pushNumeric(bytecode, tokens[i + 1]);
					i += 1;
				} else {
					std::cout << termcolor::red << "[ERROR]" << termcolor::reset <<  " Unknown structure for 'js' mnemonic on line " << opcode.lineFound
						<< ".\nExpected one of the following: \n"
						<< "  - js <number>\n"
						<< "\n";

					error = true;
				}
			} else if (opcode.valString == "jo") {
				if (tokens[i + 1].type == TokenType::NUMBER) {
					bytecode.push_back(0x37);
					pushNumeric(bytecode, tokens[i + 1]);
					i += 1;
				} else {
					std::cout << termcolor::red << "[ERROR]" << termcolor::reset <<  " Unknown structure for 'jo' mnemonic on line " << opcode.lineFound
						<< ".\nExpected one of the following: \n"
						<< "  - jo <number>\n"
						<< "\n";

					error = true;
				}
			} else if (opcode.valString == "frs") {
				bytecode.push_back(0x40);
			} else if (opcode.valString == "inc") {
				if (tokens[i + 1].type == TokenType::REGISTER) {
					bytecode.push_back(0x50);
					pushRegister(bytecode, tokens[i + 1]);
					i += 1;
				} else {
					bytecode.push_back(0x52);
				}
			} else if (opcode.valString == "dec") {
				if (tokens[i + 1].type == TokenType::REGISTER) {
					bytecode.push_back(0x51);
					pushRegister(bytecode, tokens[i + 1]);
					i += 1;
				} else {
					bytecode.push_back(0x53);
				}
			} else if (opcode.valString == "call") {
				if (tokens[i + 1].type == TokenType::NUMBER) {
					bytecode.push_back(0x60);
					i += 1;
				} else {
					std::cout << termcolor::red << "[ERROR]" << termcolor::reset <<  " Unknown structure for 'call' mnemonic on line " << opcode.lineFound
						<< ".\nExpected one of the following: \n"
						<< "  - call <number>\n"
						<< "\n";

					error = true;
				}
			} else if (opcode.valString == "ret") {
				bytecode.push_back(0x61);
			}
		}

		if (error) {
			std::cout << termcolor::red << "[ERROR]" << termcolor::reset <<  " Aborting due to errors while generating executable\n\n";
			std::exit(-1);
		}

		// Section Seperation Sequence
		char SSS[4] = { 0x1d, 0x1d, 0x1d, 0x1d };

		std::ofstream file;
		file.open(fileName, std::ios::binary);

		for (int i = 0; i < definitions.size(); i++) {
			const auto& s = definitions[i].value;
			file.write(s.c_str(), s.size());
		}

		file.write(SSS, 4);

		std::string s(bytecode.begin(), bytecode.end());
		file.write(s.c_str(), s.size());

		file.close();
	}

	void assemble(std::string fileName, cxxopts::ParseResult result) {
		auto begin = std::chrono::high_resolution_clock::now();
		
		uint8_t silent = result.count("silent");
		uint8_t customOutName = 0;

		std::string outputName = "";

		if (result.count("output")) {
			customOutName = 1;
			outputName = result["output"].as<std::string>();
		}

		if (!silent) {
			std::cout << termcolor::green << "[INFO]" << termcolor::reset << " Parsing " << termcolor::green << fileName << termcolor::reset <<  "...\n\n";
		}

		// read inputs
		if (!customOutName) {
			outputName = fileName.substr(0, fileName.find(".")) + ".ccb";
		}
		
		// tokenise
		std::vector<Token> tokens = lexer(readFile(fileName));

		std::vector<Marker> markers = {};

		// filter out the definitions
		std::vector<Definition> Definitions = parseDefinitions(tokens);
		
		// post tokenizer
		postTokenizer(tokens, markers);
		
		if (!silent) {
			std::cout << termcolor::green << "[INFO]" << termcolor::reset << " Generating " << termcolor::green << outputName << termcolor::reset << "...\n\n";
		}

		if (result.count("debug")) {
			// print the tokens for debug
			std::cout << termcolor::yellow << "[DEBUG]" << termcolor::reset << " Lexical analyzer result: \n";
			printTokens(tokens);
			std::cout << "\n";
		}
		
		generateBytecode(Definitions, tokens, outputName);

		auto end = std::chrono::high_resolution_clock::now();

		if (!silent) {
			std::cout  << termcolor::green << "[INFO]" << termcolor::reset << " Success assembling "  << termcolor::green << fileName << termcolor::reset << ", took " << termcolor::green << std::chrono::duration<double, std::milli>(end - begin).count() << termcolor::reset << "ms\n\n";
		}

		return;
	}
} 
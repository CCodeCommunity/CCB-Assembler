#include <iostream>
#include <fstream>
#include <cstdlib>
#include <streambuf>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>

// g++ main.cpp -o cca && ./cca test.cca

namespace CCA {
	bool in_array(const std::string &value, const std::vector<std::string> &array)
	{
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
	};

	struct Definition {
		int index;
		std::string value;
		std::string name;
	};
	
	std::string readFile(std::string &fileName) {
		std::ifstream file(fileName);
		std::string content;

		if (!file.is_open()) {
			std::cout << "Could not open file '" << fileName << "'\n";
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

		for (unsigned int readingIndex = 0; readingIndex < code.size(); readingIndex++) {
			char currentCharacter = code[readingIndex];

			if (currentCharacter == '\n') {
				lineFound++;
			}

			if (isIgnorable(currentCharacter)) {
				continue;
			}
			
			else if (isMarker(currentCharacter)) {
				std::string value = parseWord(code, readingIndex);

				tokens.push_back(Token {
					TokenType::MARKER,
					lineFound,
					value,
					0
				});
			}
			
			else if (isDivider(currentCharacter)) {
				tokens.push_back(Token {
					TokenType::DIVIDER,
					lineFound,
					",",
					0
				});
			}
			
			else if (isIdentifier(currentCharacter)) {
				std::string value = parseWord(code, readingIndex);

				tokens.push_back(Token {
					TokenType::IDENTIFIER,
					lineFound,
					value,
					0
				});
			}
			
			else if (isNumber(currentCharacter)) {
				int value = parseNumber(code, readingIndex);

				tokens.push_back(Token {
					TokenType::NUMBER,
					lineFound,
					"",
					value
				});
			}
			
			else if (isAddress(currentCharacter)) {
				++readingIndex;
				int value = parseNumber(code, readingIndex);

				tokens.push_back(Token {
					TokenType::ADDRESS,
					lineFound,
					"",
					value
				});
			}
			
			else if (isString(currentCharacter)) {
				++readingIndex;
				std::string value = parseString(code, readingIndex);

				tokens.push_back(Token {
					TokenType::STRING,
					lineFound,
					value,
					0
				});
			}

			else if (isComment(currentCharacter)) {
				++readingIndex;
				++lineFound;

				while (code[readingIndex] != '\n') {
					++readingIndex;
				}
			}
			
			else {
				std::cout << "[ERROR] Unexpected symbol on line " << lineFound << ": '" << currentCharacter << "'\n";
				error = true;
			}
		}

		if (error) {
			std::cout << "[ERROR] Aborting due to errors while parsing\n";
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
					std::cout << "[ERROR] error in definition statement on line " << t.lineFound << "\n";
					std::exit(-1);
				}
				
				definitions.push_back(Definition {
					definitionMemoryIndex,
					tokens[i + 2].valString,
					tokens[i + 1].valString
				});

				i += 2;
				continue;
			}

			tempTokens.push_back(t);
		}

		tokens = tempTokens;

		return definitions;
	}

	void postTokenizer(std::vector<Token> &tokens) {
		for (unsigned int i = 0; i < tokens.size(); i++) {
			Token &t = tokens[i];

			std::vector<std::string> opcodes = { "mov", "stp", "syscall", "psh", "pop", "dup", "add", "sub", "mul", "div", "not", "and", "or", "xor", "jmp", "je", "jne", "jg", "js", "jo", "frs", "inc", "dec", "call", "ret" };
			std::vector<std::string> registers = { "a", "b", "c", "d" };

			if (t.type == TokenType::IDENTIFIER && in_array(t.valString, opcodes))
				t.type = TokenType::OPCODE;

			if (t.type == TokenType::IDENTIFIER && in_array(t.valString, registers))
				t.type = TokenType::REGISTER;
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

	void generateBytecode(std::vector<Definition> definitions, std::vector<Token> tokens, std::string fileName) {
		std::vector<unsigned char> bytecode;

		bool error = false;
		
		for (unsigned int i = 0; i < tokens.size(); i++) {
			const Token &opcode = tokens[i];

			if (opcode.type != TokenType::OPCODE) {
				std::cout << "[error] expected opcode on line " << opcode.lineFound << " got " << stringifyToken(opcode.type) << ": " << stringifyTokenValue(opcode) << "\n";
				std::exit(-1);
			} else if (opcode.valString == "stp") {
				bytecode.push_back(0x00);
			} else if (opcode.valString == "syscall") {
				bytecode.push_back(0xff);
			} else if (opcode.valString == "psh") {
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
					std::cout << "[ERROR] Unknown structure for 'psh' mnemonic on line " << opcode.lineFound
						<< ".\nExpected one of the following: \n"
						<< "  - psh <number>\n"
						<< "  - psh <register>\n"
						<< "  - psh <address>\n"
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
					std::cout << "[ERROR] Unknown structure for 'pop' mnemonic on line " << opcode.lineFound
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
				} else {
					std::cout << "[ERROR] Unknown structure for 'mov' mnemonic on line " << opcode.lineFound
						<< ".\nExpected one of the following: \n"
						<< "  - mov <register>, <number>\n"//
						<< "  - mov <address>, <number>\n"//
						<< "  - mov <register>, <address>\n"//
						<< "  - mov <address>, <register>\n"
						<< "  - mov <register>, <register>\n"
						<< "  - mov <address>, <address>\n"
						<< "\n";

					error = true;
				}

				i += 3;
			}
		}

		if (error) {
			std::cout << "[ERROR] Aborting due to errors while generating executable\n\n";
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

	void assemble(std::string fileName) {
		auto begin = std::chrono::high_resolution_clock::now();
		
		std::cout << "[INFO] Parsing " << fileName << "...\n\n";

		// read inputs
		std::string outputName = fileName.substr(0, fileName.find(".")) + ".ccb";

		// tokenise
		std::vector<Token> tokens = lexer(readFile(fileName));

		// post tokenizer
		postTokenizer(tokens);

		// filter out the definitions
		std::vector<Definition> Definitions = parseDefinitions(tokens);
		
		std::cout << "[INFO] Generating " << outputName << "...\n\n";

		// print the tokens for debug
		// printTokens(tokens);

		generateBytecode(Definitions, tokens, outputName);

		auto end = std::chrono::high_resolution_clock::now();

		std::cout << "[INFO] Success assembling " << fileName << ", took " << std::chrono::duration<double, std::milli>(end - begin).count() << "ms\n\n";

		return;
	}
} 
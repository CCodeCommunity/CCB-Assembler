#include <iostream>
#include <fstream>
#include <cstdlib>
#include <streambuf>
#include <string>
#include <vector>

// g++ main.cpp -o cca && ./cca test.cca

namespace CCA {
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
	
	std::string readFile(std::string fileName) {
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

	std::string parseWord(std::string code, unsigned int &readingIndex) {
		std::string result = "";
		result += code[readingIndex++];

		while(isIdentifier(code[readingIndex])) {
			result += code[readingIndex++];
		}

		--readingIndex;

		return result;
	}

	std::string parseString(std::string code, unsigned int &readingIndex) {
		std::string result = "";
		result += code[readingIndex++];

		while(!isString(code[readingIndex])) {
			result += code[readingIndex++];
		}

		return result;
	}

	int parseNumber(std::string code, unsigned int &readingIndex) {
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
		int lineFound = 0;

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
					TokenType::NUMBER,
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
			
			else {
				std::cout << "[ERROR] unexpected symbol on line " << lineFound << " '" << currentCharacter << "'\n";
			}
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

	void parseDefinitions(std::vector<Token> &tokens) {
		for (auto &t: tokens) {
			if (t.type == TokenType::IDENTIFIER && t.valString == "def") {
				std::cout << "found definition\n";
			}
		}
	}

	void assemble(std::string fileName) {
		std::cout << "Assembling " << fileName << "...\n";

		std::string outputName = fileName.substr(0, fileName.find(".")) + ".ccb";

		std::vector<Token> tokens = lexer(readFile(fileName));

		std::cout << "Generating " << outputName << "...\n";

		printTokens(tokens);

		return;
	}
}

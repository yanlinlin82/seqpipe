#include <iostream>
#include "CommandLineParser.h"
#include "System.h"

int CommandLineParser::ParseHex(int c)
{
	if (c >= '0' && c <= '9') {
		return c - '0';
	} else if (c >= 'A' && c <= 'F') {
		return c - 'A' + 10;
	} else if (c >= 'a' && c <= 'f') {
		return c - 'a' + 10;
	} else {
		return -1;
	}
}

int CommandLineParser::ParseOct(int c)
{
	if (c >= '0' && c <= '7') {
		return c - '0';
	} else {
		return -1;
	}
}

int CommandLineParser::ParseHex(int c1, int c2)
{
	int v1 = ParseHex(c1);
	if (v1 < 0) {
		return -1;
	}
	int v2 = ParseHex(c2);
	if (v2 < 0) {
		return -1;
	}
	return v1 * 16 + v2;
}

int CommandLineParser::ParseOct(int c1, int c2)
{
	int v1 = ParseOct(c1);
	if (v1 < 0) {
		return -1;
	}
	int v2 = ParseOct(c2);
	if (v2 < 0) {
		return -1;
	}
	return v1 * 8 + v2;
}

bool CommandLineParser::Parse(const std::string& s)
{
	argLists_.clear();
	argLists_.resize(1);

	std::string word;
	for (size_t i = 0; i < s.size(); ++i) {
		if (s[i] == ';') {
			if (!word.empty()) {
				argLists_.back().push_back(word);
				word = "";
			}
			argLists_.resize(argLists_.size() + 1);
		} else if (isspace(s[i])) { // space, '\t', '\r', '\n', '\f', '\v'
			if (!word.empty()) {
				argLists_.back().push_back(word);
				word = "";
			}
		} else if (s[i] == '\'') {
			for (++i; i < s.size(); ++i) {
				if (s[i] == '\'') {
					break;
				}
				word += s[i];
			}
			if (i >= s.size()) {
				return false;
			}
		} else if (s[i] == '"') {
			for (++i; i < s.size(); ++i) {
				if (s[i] == '"') {
					break;
				} else if (s[i] == '\\') {
					if (i + 1 >= s.size()) {
						return false;
					}
					++i;
					if (s[i] == 'x') {
						if (i + 2 >= s.size()) {
							return false;
						}
						int value = ParseHex(s[i + 1], s[i + 2]);
						if (value < 0) {
							return false;
						}
						i += 2;
						word += static_cast<char>(value);
					} else if (s[i] == '0') {
						if (i + 2 >= s.size()) {
							return false;
						}
						int value = ParseOct(s[i + 1], s[i + 2]);
						if (value < 0) {
							return false;
						}
						i += 2;
						word += static_cast<char>(value);
					} else if (s[i] == 't' || s[i] == 'r' || s[i] == 'n' || s[i] == 'b') {
						word += '\\';
						word += s[i];
					} else {
						return false;
					}
				} else {
					word += s[i];
				}
			}
			if (i >= s.size()) {
				return false;
			}
		} else if (s[i] == '\\') {
			word += s[i];
			if (i + 1 >= s.size()) {
				return false;
			}
			word += s[++i];
		} else {
			word += s[i];
		}
	}
	if (!word.empty()) {
		argLists_.back().push_back(word);
	}

	//Dump();
	return true;
}

std::string CommandLineParser::ToFullCmdLine() const
{
	std::string s;
	for (size_t i = 0; i < argLists_.size(); ++i) {
		if (i > 0) {
			s += "; ";
		}
		for (size_t j = 0; j < argLists_[i].size(); ++j) {
			if (j > 0) {
				s += " ";
			}
			s += System::EncodeShell(argLists_[i][j]);
		}
	}
	return s;
}

void CommandLineParser::Dump() const
{
	std::cerr << "Total " << argLists_.size() << " args:\n";
	for (size_t i = 0; i < argLists_.size(); ++i) {
		std::cerr << "[" << i << "]";
		for (const auto& x : argLists_[i]) {
			std::cerr << " (" <<  x << ")";
		}
		std::cerr << "\n";
	}
	std::cerr << std::flush;
}

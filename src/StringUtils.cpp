#include <iostream>
#include <deque>
#include "StringUtils.h"

static int ParseHex(int c)
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

static int ParseOct(int c)
{
	if (c >= '0' && c <= '7') {
		return c - '0';
	} else {
		return -1;
	}
}

static int ParseHex(int c1, int c2)
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

static int ParseOct(int c1, int c2)
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

bool StringUtils::ParseCommandLine(const std::string& s, std::string& cmd, std::vector<std::string>& arguments)
{
	std::deque<std::string> args;

	std::string word;
	for (size_t i = 0; i < s.size(); ++i) {
		if (isspace(s[i])) { // space, '\t', '\r', '\n', '\f', '\v'
			if (!word.empty()) {
				args.push_back(word);
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
		args.push_back(word);
	}

	if (args.empty()) {
		return false;
	}

	cmd = args.front();
	args.pop_front();
	arguments = std::vector<std::string>(args.begin(), args.end());
	return true;
}

std::string StringUtils::RemoveSpecialCharacters(const std::string& s)
{
	std::string t;
	for (size_t i = 0; i < s.size(); ++i) {
		if (s[i] == '-' || s[i] == '_' || s[i] == '+' ||
				(s[i] >= '0' && s[i] <= '9') ||
				(s[i] >= 'A' && s[i] <= 'Z') ||
				(s[i] >= 'a' && s[i] <= 'z')) {
			t += s[i];
		} else if (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n') {
			break;
		}
	}
	return t;
}

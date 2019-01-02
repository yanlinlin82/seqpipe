#pragma once
#include <string>
#include <vector>
#include <regex>

class StringUtils
{
public:
	static const std::string SPACES;

	static std::string TrimLeft(const std::string& s, const std::string& spaces = SPACES);
	static std::string TrimRight(const std::string& s, const std::string& spaces = SPACES);
	static std::string Trim(const std::string& s, const std::string& spaces = SPACES);

	static std::string RemoveSpecialCharacters(const std::string& s);

	static std::vector<std::string> Split(const std::string& s, const std::string& sep);
	static std::vector<std::string> Split(const std::string& s, const std::regex& sep);

	static std::string SingleQuote(const std::string& s);
	static std::string DoubleQuote(const std::string& s);

	static std::string ShellQuote(const std::string& s, bool singleQuote);
};

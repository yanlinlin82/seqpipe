#include <iostream>
#include <deque>
#include "StringUtils.h"

std::string StringUtils::TrimLeft(const std::string& s)
{
	std::string::size_type pos = s.find_first_not_of(" \t\r\n");
	return (pos == std::string::npos ? s : s.substr(pos));
}

std::string StringUtils::TrimRight(const std::string& s)
{
	std::string::size_type pos = s.find_last_not_of(" \t\r\n");
	return (pos == std::string::npos ? s : s.substr(0, pos + 1));
}

std::string StringUtils::Trim(const std::string& s)
{
	return TrimLeft(TrimRight(s));
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

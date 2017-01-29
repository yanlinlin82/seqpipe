#ifndef STRING_UTILS_H__
#define STRING_UTILS_H__

#include <string>
#include <vector>
#include <ctime>

class StringUtils
{
public:
	static std::string TrimLeft(const std::string& s);
	static std::string TrimRight(const std::string& s);
	static std::string Trim(const std::string& s);

	static std::string RemoveSpecialCharacters(const std::string& s);
	static bool ParseCommandLine(const std::string& s, std::string& cmd, std::vector<std::string>& arguments);
};

#endif

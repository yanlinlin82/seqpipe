#ifndef STRING_UTILS_H__
#define STRING_UTILS_H__

#include <string>
#include <vector>
#include <ctime>

class StringUtils
{
public:
	static std::string GetFirstWord(const std::string& s);
	static std::string TimeString(time_t t);
	static std::string DiffTimeString(int elapsed);
	static std::string RemoveSpecialCharacters(const std::string& s);
	static bool ParseCommandLine(const std::string& s, std::string& cmd, std::vector<std::string>& arguments);
};

#endif

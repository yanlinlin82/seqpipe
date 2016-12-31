#ifndef STRING_UTILS_H__
#define STRING_UTILS_H__

#include <string>
#include <ctime>

class StringUtils
{
public:
	static std::string TimeString(time_t t);
	static std::string DiffTimeString(int elapsed);
	static std::string RemoveSpecialCharacters(const std::string& s);
};

#endif

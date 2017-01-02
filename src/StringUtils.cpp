#include "StringUtils.h"

std::string StringUtils::GetFirstWord(const std::string& s)
{
	std::string::size_type pos = s.find_first_of(" \t\n\r");
	if (pos != std::string::npos) {
		return s.substr(0, pos);
	}
	return s;
}

std::string StringUtils::TimeString(time_t t)
{
	tm tmBuf;
	localtime_r(&t, &tmBuf);
	char buffer[32] = "";
	snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
			tmBuf.tm_year + 1900, tmBuf.tm_mon + 1, tmBuf.tm_mday,
			tmBuf.tm_hour, tmBuf.tm_min, tmBuf.tm_sec);
	return buffer;
}

std::string StringUtils::DiffTimeString(int elapsed)
{
	std::string s;

	if (elapsed >= 86400) {
		s += std::to_string(elapsed / 86400) + "d";
		elapsed %= 86400;
	}
	if (elapsed >= 3600) {
		s += (s.empty() ? "" : " ") + std::to_string(elapsed / 3600) + "h";
		elapsed %= 3600;
	}
	if (elapsed >= 60) {
		s += (s.empty() ? "" : " ") + std::to_string(elapsed / 60) + "m";
		elapsed %= 60;
	}
	if (s.empty() || elapsed > 0) {
		s += (s.empty() ? "" : " ") + std::to_string(elapsed) + "s";
	}
	return s;
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

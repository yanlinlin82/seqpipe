#include "TimeString.h"

std::string TimeString(time_t t)
{
	tm buf;
	localtime_r(&t, &buf);
	char text[128] = "";
	snprintf(text, sizeof(text), "%04d-%02d-%02d %02d:%02d:%02d",
			buf.tm_year + 1900, buf.tm_mon + 1, buf.tm_mday,
			buf.tm_hour, buf.tm_min, buf.tm_sec);
	return text;
}

#include "LauncherTimer.h"

static std::string TimeString(time_t t)
{
	tm buf;
	localtime_r(&t, &buf);
	char text[32] = "";
	snprintf(text, sizeof(text), "%04d-%02d-%02d %02d:%02d:%02d",
			buf.tm_year + 1900, buf.tm_mon + 1, buf.tm_mday,
			buf.tm_hour, buf.tm_min, buf.tm_sec);
	return text;
}

std::string LauncherTimer::StartTime() const
{
	return TimeString(start_);
}

std::string LauncherTimer::EndTime() const
{
	return TimeString(end_);
}

std::string LauncherTimer::Elapse() const
{
	int elapsed = end_ - start_;

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

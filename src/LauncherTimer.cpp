#include <cassert>
#include "LauncherTimer.h"
#include "TimeString.h"

std::string LauncherTimer::StartTime() const
{
	assert(start_ != -1);
	return TimeString(start_);
}

std::string LauncherTimer::EndTime() const
{
	assert(end_ != -1);
	return TimeString(end_);
}

std::string LauncherTimer::Elapse() const
{
	assert(start_ != -1);
	assert(end_ != -1);

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

#ifndef LAUNCHER_TIMER_H__
#define LAUNCHER_TIMER_H__

#include <string>
#include <ctime>

class LauncherTimer
{
public:
	LauncherTimer() : t0_(time(NULL)) { }
public:
	void Stop() { t_ = time(NULL); }
	std::string StartTime() const { return StringUtils::TimeString(t0_); }
	std::string EndTime() const { return StringUtils::TimeString(t_); }
	std::string Elapse() const { return StringUtils::DiffTimeString(t_ - t0_); }
private:
	time_t t0_;
	time_t t_;
};

#endif

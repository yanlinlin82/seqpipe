#ifndef LAUNCHER_TIMER_H__
#define LAUNCHER_TIMER_H__

#include <string>
#include <ctime>

class LauncherTimer
{
public:
	LauncherTimer() : start_(time(NULL)), end_(-1) { }

	void Start() { start_ = time(NULL); }
	void Stop() { end_ = time(NULL); }

	std::string StartTime() const;
	std::string EndTime() const;
	std::string Elapse() const;
private:
	time_t start_;
	time_t end_;
};

#endif

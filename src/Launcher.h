#ifndef LAUNCHER_H__
#define LAUNCHER_H__

#include <list>
#include <string>
#include <vector>

class Launcher
{
public:
	int Run(const std::list<std::string>& args);
private:
	bool ParseArgs(const std::list<std::string>& args);
	void PrintUsage();
	bool WriteToHistoryLog(const std::string& uniqueId);
	bool CreateLastSymbolicLink(const std::string& uniqueId);
private:
	int verbose_ = 0;
	int maxJobNumber_ = 0;
	bool forceRun_ = false;
	bool keepTemp_ = false;

	std::string command_;
	bool commandIsPipeFile_ = false;
	std::vector<std::string> arguments_;

	std::string log_;
	size_t counter_;
};

#endif

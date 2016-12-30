#ifndef COMMAND_HISTORY_H__
#define COMMAND_HISTORY_H__

#include <list>
#include <string>

class CommandHistory
{
public:
	int Run(const std::list<std::string>& args);
private:
	bool ParseArgs(const std::list<std::string>& args);
	void PrintUsage();
private:
	int verbose_ = 0;
};

#endif

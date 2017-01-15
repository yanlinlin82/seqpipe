#ifndef COMMAND_RUN_DEPRECATED_H__
#define COMMAND_RUN_DEPRECATED_H__

#include <list>
#include <string>
#include <vector>
#include "Launcher.h"

class CommandRunDeprecated
{
public:
	int Run(const std::list<std::string>& args);
private:
	bool ParseArgs(const std::list<std::string>& args);
	void PrintUsage();
private:
	int helpMode_ = 0;
	int verbose_ = 0;
};

#endif


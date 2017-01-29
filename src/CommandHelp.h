#ifndef COMMAND_HELP_H__
#define COMMAND_HELP_H__

#include <string>
#include <list>

class CommandHelp
{
public:
	int Run(const std::list<std::string>& args);
private:
	void PrintUsage();
};

#endif

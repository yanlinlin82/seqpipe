#ifndef COMMAND_HELP_H__
#define COMMAND_HELP_H__

#include <string>
#include <vector>

class CommandHelp
{
public:
	int Run(const std::vector<std::string>& args);
private:
	void PrintUsage();
};

#endif

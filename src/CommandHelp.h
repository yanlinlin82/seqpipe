#pragma once
#include <string>
#include <vector>

class CommandHelp
{
public:
	int Run(const std::vector<std::string>& args);
private:
	void PrintUsage();
};

#pragma once
#include <vector>
#include <string>

class CommandLog
{
public:
	int Run(const std::vector<std::string>& args);
private:
	bool ParseArgs(const std::vector<std::string>& args);
	void PrintUsage();
	int ListHistory();
	int ShowHistory();
	int RemoveHistory();
	bool RelinkLastSymbolic(const std::string& uniqueId);
private:
	int verbose_ = 0;
	std::string command_;
	std::string idOrOrder_;
};

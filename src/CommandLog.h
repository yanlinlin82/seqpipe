#ifndef COMMAND_LOG_H__
#define COMMAND_LOG_H__

#include <list>
#include <string>

class CommandLog
{
public:
	int Run(const std::list<std::string>& args);
private:
	bool ParseArgs(const std::list<std::string>& args);
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

#endif

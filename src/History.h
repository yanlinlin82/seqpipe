#ifndef HISTORY_H__
#define HISTORY_H__

#include <list>
#include <string>

class History
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

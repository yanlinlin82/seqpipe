#pragma once
#include <string>
#include <vector>
#include "Pipeline.h"

class CommandRun
{
public:
	int Run(const std::vector<std::string>& args);
private:
	bool ParseArgs(const std::vector<std::string>& args);
	void PrintUsage();
	void ListModules();
private:
	int helpMode_ = 0;
	int listMode_ = 0;
	int verbose_ = 0;
	int maxJobNumber_ = 0;
	bool forceRun_ = false;
	bool keepTemp_ = false;

	ProcArgs procArgs_;
	Pipeline pipeline_;
	std::string pattern_; // for '-l' or '-L'
};

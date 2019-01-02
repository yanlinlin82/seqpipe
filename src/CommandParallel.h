#pragma once
#include <string>
#include <vector>
#include "Pipeline.h"

class CommandParallel
{
public:
	int Run(const std::vector<std::string>& args);
private:
	void PrintUsage();
	bool ParseArgs(const std::vector<std::string>& args);

	bool LoadCmdLineList(const std::string& filename, std::vector<std::string>& cmdList);
private:
	int verbose_ = 0;
	int maxJobNumber_ = 0;

	ProcArgs procArgs_;
	Pipeline pipeline_;
};

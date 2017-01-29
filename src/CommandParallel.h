#ifndef COMMAND_PARALLEL_H__
#define COMMAND_PARALLEL_H__

#include <string>
#include <vector>
#include "Pipeline.h"

class CommandParallel
{
public:
	int Run(const std::vector<std::string>& args);
private:
	void PrintUsage();
	bool LoadCommandList(const std::string& filename);
	bool ParseArgs(const std::vector<std::string>& args);
private:
	int verbose_ = 0;
	int maxJobNumber_ = 0;

	std::string commandFilename_;
	std::vector<std::string> commandList_;
	Pipeline pipeline_;
};

#endif

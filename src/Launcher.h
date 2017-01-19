#ifndef LAUNCHER_H__
#define LAUNCHER_H__

#include <string>
#include <vector>
#include "LogFile.h"

class CommandItem
{
public:
	std::string name_;
	std::string cmdLine_;
};

class Launcher
{
public:
	static bool CheckIfPipeFile(const std::string& command);
	bool LoadPipeFile(const std::string& filename);
	bool WritePipeFile(const std::string& filename) const;

	bool AppendCommand(const std::string&  cmd, const std::vector<std::string>& arguments);
	std::string JoinCommandLine(const std::string& cmd, const std::vector<std::string>& arguments);

	int Run(LogFile& logFile, const std::string& logDir, int verbose);
	std::vector<std::string> GetModules() const;
private:
	std::vector<std::string> originPipeline_;
	std::vector<CommandItem> commandLines_;
	std::vector<std::string> modules_;
};

#endif

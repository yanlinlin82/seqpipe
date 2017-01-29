#ifndef LAUNCHER_H__
#define LAUNCHER_H__

#include <string>
#include <vector>
#include "LauncherCounter.h"
#include "LogFile.h"
#include "Pipeline.h"

class Launcher
{
public:
	int Run(const Pipeline& pipeline, const std::string& procName, int verbose);
private:
	int RunProc(const Pipeline& pipeline, const std::string& procName, std::string indent);
	int RunBlock(const Pipeline& pipeline, const Block& block, std::string indent);
	int RunShell(const CommandItem& item, std::string indent);

	bool WriteToHistoryLog();
	bool CreateLastSymbolicLink();
	bool PrepareToRun();
	bool RecordSysInfo(const std::string& filename);

	static std::string GetUniqueId();
private:
	std::string uniqueId_;
	LauncherCounter counter_;
	LogFile logFile_;
	std::string logDir_;
	int verbose_;
};

#endif

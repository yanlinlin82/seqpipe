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
	Launcher(const Pipeline& pipeline, int verbose);

	int Run(const ProcArgs& procArgs);
private:
	int RunProc(const std::string& procName, std::string indent, const ProcArgs& procArgs);
	int RunBlock(const Block& block, std::string indent, const ProcArgs& procArgs);
	int RunShell(const CommandItem& item, std::string indent);

	bool WriteToHistoryLog();
	bool CreateLastSymbolicLink();
	bool PrepareToRun();
	bool RecordSysInfo(const std::string& filename);

	static std::string GetUniqueId();
private:
	const Pipeline& pipeline_;

	std::string uniqueId_;
	LauncherCounter counter_;
	LogFile logFile_;
	std::string logDir_;
	int verbose_;
};

#endif

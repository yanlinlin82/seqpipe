#ifndef LAUNCHER_H__
#define LAUNCHER_H__

#include <string>
#include <vector>
#include <mutex>
#include "LogFile.h"
#include "Pipeline.h"

class LauncherCounter
{
public:
	unsigned int FetchID();
private:
	std::mutex mutex_;
	unsigned int counter_ = 0;
};

class Launcher
{
public:
	int Run(const Pipeline& pipeline, const std::string& procName, int verbose);
private:
	int RunProc(const Pipeline& pipeline, const std::string& procName, LogFile& logFile, const std::string& logDir, std::string indent, int verbose);
	int RunBlock(const Pipeline& pipeline, const Block& block, LogFile& logFile, const std::string& logDir, std::string indent, int verbose);
	int RunShell(const CommandItem& item, LogFile& logFile, const std::string& logDir, std::string indent, int verbose);

	bool WriteToHistoryLog(const std::string& uniqueId);
	bool CreateLastSymbolicLink(const std::string& uniqueId);
	bool PrepareToRun(const std::string& logDir, const std::string& uniqueId);
	bool RecordSysInfo(const std::string& filename);
private:
	LauncherCounter counter_;
};

#endif

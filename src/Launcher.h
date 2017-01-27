#ifndef LAUNCHER_H__
#define LAUNCHER_H__

#include <string>
#include <vector>
#include "LogFile.h"
#include "Pipeline.h"

class Launcher
{
public:
	int Run(const Pipeline& pipeline, int verbose);
private:
	int Run(const Pipeline& pipeline, LogFile& logFile, const std::string& logDir, int verbose);
	bool WriteToHistoryLog(const std::string& uniqueId);
	bool CreateLastSymbolicLink(const std::string& uniqueId);
	bool PrepareToRun(const std::string& logDir, const std::string& uniqueId);
	bool RecordSysInfo(const std::string& filename);
};

#endif

#pragma once
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "LauncherTimer.h"
#include "LogFile.h"
#include "Pipeline.h"

class ShellTask
{
public:
	ShellTask() { }
	ShellTask(unsigned int shellId, const std::string& name, const std::string& cmdLine, const std::string& indent, unsigned int taskId):
		shellId_(shellId), name_(name), cmdLine_(cmdLine), indent_(indent), taskId_(taskId)
	{ }

	int Run(LogFile& logFile, const std::string& logDir, int verbose_);
	unsigned int GetTaskId() const { return taskId_; }
private:
	unsigned int shellId_ = 0;
	std::string name_;
	std::string cmdLine_;
	std::string indent_;
	unsigned int taskId_ = 0;
};

class Task
{
public:
	Task() { }
	Task(const Statement* block, size_t itemIndex, std::string indent, ProcArgs procArgs, unsigned int taskId):
		block_(block), itemIndex_(itemIndex), indent_(indent), procArgs_(procArgs), taskId_(taskId)
	{
	}

	void SetEnd(LogFile& logFile, unsigned int taskId, int retVal);

	const Statement* block_ = nullptr;
	size_t itemIndex_ = 0;
	std::string indent_ = "";
	ProcArgs procArgs_;

	bool finished_ = false;
	std::set<unsigned int> waitingFor_;
	int retVal_ = 0;

	LauncherTimer timer_;
	unsigned int shellId_ = 0;
	unsigned int taskId_ = 0;
};

class Launcher
{
public:
	Launcher(const Pipeline& pipeline, int maxJobNumber, int verbose);

	int Run(const ProcArgs& procArgs);
private:
	int RunShell(unsigned int shellId, const Statement& item, std::string indent, const ProcArgs& procArgs);

	bool WriteToHistoryLog();
	bool CreateLastSymbolicLink();
	bool PrepareToRun();
	bool RecordSysInfo(const std::string& filename);

	static std::string GenerateSessionId();
private:
	const Pipeline& pipeline_;

	unsigned int shellIdCounter_ = 0;
	unsigned int taskIdCounter_ = 0;

	std::string sessionId_;
	LogFile logFile_;
	std::string logDir_;
	int verbose_;

private:
	void Worker();
	void PostTask(const Statement& block, Task& info, const std::string& indent, const ProcArgs& procArgs);
private:
	int maxJobNumber_ = 0;

	std::mutex shellTaskQueueMutex_;
	std::condition_variable shellTaskQueueCondVar_;
	std::list<ShellTask> shellTaskQueue_;

	std::mutex mutex_;
	std::list<Task> runningTasks_;
	std::map<unsigned int, int> finishedTasks_; // { task-id => exit-value }
	std::vector<int> failedRetVal_;
};

#pragma once
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "LauncherCounter.h"
#include "LauncherTimer.h"
#include "LogFile.h"
#include "Pipeline.h"

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

	unsigned int taskId_ = 0;

	bool finished_ = false;
	std::set<unsigned int> waitingFor_;
	int retVal_ = 0;

	LauncherTimer timer_;
	unsigned int id_ = 0;
};

class Launcher
{
public:
	Launcher(const Pipeline& pipeline, int maxJobNumber, int verbose);

	int Run(const ProcArgs& procArgs);
private:
	int RunShell(const Statement& item, std::string indent, const ProcArgs& procArgs);

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

private:
	void ShellWorker();
	void PostTask(const Statement& block, Task& info, const std::string& indent, const ProcArgs& procArgs);
	void RunShellTask(const Task& shellTask);

	void Worker();
	void RunTask(Task& task);
private:
	int maxJobNumber_ = 0;

	std::mutex shellTaskQueueMutex_;
	std::condition_variable shellTaskQueueCondVar_;
	std::list<Task> shellTaskQueue_;

	std::mutex mutex_;
	unsigned int taskIdCounter_ = 0;
	std::list<Task> runningTasks_;
	std::map<unsigned int, int> finishedTasks_; // { task-id => exit-value }
	std::vector<int> failedRetVal_;

	std::mutex mutex2_;
	std::condition_variable cv2_;
	Task rootTask_;
};

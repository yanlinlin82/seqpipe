#ifndef LAUNCHER_H__
#define LAUNCHER_H__

#include <string>
#include <vector>
#include <list>
#include <map>
#include <atomic>
#include <mutex>
#include <set>
#include "LauncherCounter.h"
#include "LogFile.h"
#include "Pipeline.h"

class WorkflowTask
{
public:
	WorkflowTask() { }
	WorkflowTask(size_t blockIndex, size_t itemIndex, std::string indent, ProcArgs procArgs, unsigned int taskId);

	size_t blockIndex_ = 0;
	size_t itemIndex_ = 0;
	std::string indent_ = "";
	ProcArgs procArgs_;

	unsigned int taskId_ = 0;
};

class WorkflowThread
{
public:
	WorkflowThread(size_t blockIndex, size_t itemIndex, std::string indent, ProcArgs procArgs, unsigned int taskId);

	size_t blockIndex_ = 0;
	size_t itemIndex_ = 0;
	std::string indent_ = "";
	ProcArgs procArgs_;

	unsigned int taskId_ = 0;

	std::set<unsigned int> waitingFor_;
	int retVal_ = 0;
};

class Launcher
{
public:
	Launcher(const Pipeline& pipeline, int maxJobNumber, int verbose);

	int Run(const ProcArgs& procArgs);
private:
	int ProcessWorkflowThreads(const ProcArgs& procArgs);

	int RunProc(const std::string& procName, const ProcArgs& procArgs, std::string indent);
	int RunBlock(const Block& block, const ProcArgs& procArgs, std::string indent);
	int RunShell(const CommandItem& item, std::string indent, const ProcArgs& procArgs);

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
	enum Status { STATUS_RUNNING, STATUS_EXITED };

	Status CheckStatus();
	void Wait();
	void CheckFinishedTasks();
	void PostNextTasks();
	void EraseFinishedThreads();
	void DumpWorkflowThreads() const;

	void Worker();
	bool GetTaskFromQueue(WorkflowTask& task);
	void SetTaskFinished(unsigned int taskId, int retVal);
private:
	int maxJobNumber_ = 0;
	std::mutex mutex_;
	std::list<WorkflowThread> workflowThreads_;
	unsigned int taskIdCounter_ = 0;
	std::list<WorkflowTask> taskQueue_;
	std::map<unsigned int, int> finishedTasks_; // { task-id => exit-value }
	std::vector<int> failedRetVal_;
	std::atomic<bool> exiting_{false};
};

#endif

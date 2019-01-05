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

class WorkflowTask
{
public:
	WorkflowTask() { }
	WorkflowTask(const Statement* block, size_t itemIndex, std::string indent, ProcArgs procArgs, unsigned int taskId):
		block_(block), itemIndex_(itemIndex), indent_(indent), procArgs_(procArgs), taskId_(taskId)
	{
	}

	const Statement* block_ = nullptr;
	size_t itemIndex_ = 0;
	std::string indent_ = "";
	ProcArgs procArgs_;

	unsigned int taskId_ = 0;
};

class WorkflowThread: public WorkflowTask
{
public:
	WorkflowThread(const Statement* block, size_t itemIndex, std::string indent, ProcArgs procArgs, unsigned int taskId):
		WorkflowTask(block, itemIndex, indent, procArgs, taskId)
	{
	}

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
	void CheckFinishedTasks();
	void PostNextTasks();
	void EraseFinishedThreads();

	void Worker();
	bool GetTaskFromQueue(WorkflowTask& task);
	void SetTaskFinished(unsigned int taskId, int retVal);

	void PostStatementToThreads(const Statement& block, WorkflowThread& info, std::list<WorkflowThread>& newThreads,
			const std::string& indent, const ProcArgs& procArgs);
private:
	int maxJobNumber_ = 0;
	std::mutex mutex_;
	std::list<WorkflowThread> workflowThreads_;
	unsigned int taskIdCounter_ = 0;
	std::list<WorkflowTask> taskQueue_;
	std::map<unsigned int, int> finishedTasks_; // { task-id => exit-value }
	std::vector<int> failedRetVal_;

private:
	bool WaitForTask();
	void Notify();
	void NotifyAll();
private:
	std::mutex mutexWorker_;
	std::condition_variable condWorker_;
	unsigned int countWorker_ = 0;

	std::atomic_int waitingWorker_{0};
};

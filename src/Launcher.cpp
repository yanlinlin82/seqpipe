#include <iostream>
#include <fstream>
#include <atomic>
#include <thread>
#include <csignal>
#include <unistd.h>
#include "Launcher.h"
#include "System.h"
#include "StringUtils.h"
#include "SeqPipe.h"
#include "Semaphore.h"
#include "LauncherTimer.h"

WorkflowTask::WorkflowTask(size_t blockIndex, size_t itemIndex, std::string indent, ProcArgs procArgs, unsigned int taskId):
	blockIndex_(blockIndex), itemIndex_(itemIndex), indent_(indent), procArgs_(procArgs), taskId_(taskId)
{
}

WorkflowThread::WorkflowThread(size_t blockIndex, size_t itemIndex, std::string indent, ProcArgs procArgs):
	blockIndex_(blockIndex), itemIndex_(itemIndex), indent_(indent), procArgs_(procArgs)
{
}

Launcher::Launcher(const Pipeline& pipeline, int maxJobNumber, int verbose):
	pipeline_(pipeline), maxJobNumber_(maxJobNumber), verbose_(verbose)
{
}

std::atomic<bool> killed(false);

void MySigAction(int signum, siginfo_t* siginfo, void* ucontext)
{
	killed = true;
#if 0
	time_t now = time(NULL);
	logFile_.WriteLine(Msg() << "(0) Aborts at " + StringUtils::TimeString(now));
#endif
}

static void SetSigAction()
{
	struct sigaction sa = { };
	sa.sa_sigaction = MySigAction;
	sa.sa_flags = SA_SIGINFO;

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
}

int Launcher::RunProc(const std::string& procName, const ProcArgs& procArgs, std::string indent)
{
	unsigned int id = counter_.FetchId();

	const std::string name = std::to_string(id) + "." + procName;

	logFile_.WriteLine(Msg() << indent << "(" << id << ") [pipeline] " << procName << procArgs.ToString());
	LauncherTimer timer;
	logFile_.WriteLine(Msg() << indent << "(" << id << ") starts at " << timer.StartTime());

	WriteStringToFile(logDir_ + "/" + name + ".pipeline", procName);

	int retVal = RunBlock(pipeline_.GetBlock(procName), procArgs, indent + "  ");

	timer.Stop();
	logFile_.WriteLine(Msg() << indent << "(" << id << ") ends at " << timer.EndTime() << " (elapsed: " << timer.Elapse() << ")");

	return retVal;
}

int Launcher::RunShell(const CommandItem& item, std::string indent)
{
	unsigned int id = counter_.FetchId();

	const std::string name = std::to_string(id) + "." + item.Name();
	const auto& cmdLine = item.CmdLine();

	logFile_.WriteLine(Msg() << indent << "(" << id << ") [shell] " << cmdLine);
	LauncherTimer timer;
	logFile_.WriteLine(Msg() << indent << "(" << id << ") starts at " << timer.StartTime());

	WriteStringToFile(logDir_ + "/" + name + ".cmd", cmdLine);

	std::string fullCmdLine = "( " + cmdLine + " )";
	if (verbose_ > 0) {
		fullCmdLine += " 2> >(tee -a " + logDir_ + "/" + name + ".err >&2)";
		fullCmdLine += " > >(tee -a " + logDir_ + "/" + name + ".log)";
	} else {
		fullCmdLine += " 2>>" + logDir_ + "/" + name + ".err";
		fullCmdLine += " >>" + logDir_ + "/" + name + ".log";
	}
	int retVal = System::Execute(fullCmdLine.c_str());

	timer.Stop();
	logFile_.WriteLine(Msg() << indent << "(" << id << ") ends at " << timer.EndTime() << " (elapsed: " << timer.Elapse() << ")");

	if (retVal != 0) {
		logFile_.WriteLine(Msg() << indent << "(" << id << ") returns " << retVal);
		return retVal;
	}
	return 0;
}

int Launcher::RunBlock(const Block& block, const ProcArgs& procArgs, std::string indent)
{
	for (size_t i = 0; i < block.items_.size() && !killed; ++i) {
		const auto item = block.items_[i];
		int retVal;
		if (item.Type() == CommandItem::TYPE_PROC) {
			retVal = RunProc(item.ProcName(), item.GetProcArgs(), indent);
		} else if (item.Type() == CommandItem::TYPE_SHELL) {
			retVal = RunShell(item, indent);
		}
		if (retVal != 0) {
			return retVal;
		}
	}
	return 0;
}

std::string Launcher::GetUniqueId()
{
	char text[64] = "";
	time_t now = time(NULL);
	struct tm buf;
	localtime_r(&now, &buf);
	snprintf(text, sizeof(text), "%02d%02d%02d.%02d%02d.%d.",
			buf.tm_year % 100, buf.tm_mon + 1, buf.tm_mday,
			buf.tm_hour, buf.tm_min, getpid());
	return (text + System::GetHostname());
}

Launcher::Status Launcher::CheckStatus()
{
	std::lock_guard<std::mutex> lock(mutex_);

	CheckFinishedTasks();
	PostNextTasks();
	EraseFinishedThreads();

	return (workflowThreads_.empty() ? STATUS_EXITED : STATUS_RUNNING);
}

void Launcher::Wait()
{
	usleep(100);
}

void Launcher::CheckFinishedTasks()
{
	for (auto& info : workflowThreads_) {
		if (info.waitingFor_ > 0) {
			const auto& block = pipeline_.GetBlock(info.blockIndex_);
			auto it = finishedTasks_.find(info.waitingFor_);
			if (it != finishedTasks_.end()) {
				finishedTasks_.erase(info.waitingFor_);
				info.waitingFor_ = 0;
				info.retVal_ = it->second;
				if (block.parallel_) {
					info.itemIndex_ = block.items_.size();
				} else {
					++info.itemIndex_;
				}
			}
		}
	}
}

void Launcher::PostNextTasks()
{
	for (auto& info : workflowThreads_) {
		const auto& block = pipeline_.GetBlock(info.blockIndex_);
		if (info.waitingFor_ == 0 && info.retVal_ == 0 && info.itemIndex_ < block.items_.size()) {
			++taskIdCounter_;
			taskQueue_.push_back(WorkflowTask(info.blockIndex_, info.itemIndex_, info.indent_, info.procArgs_, taskIdCounter_));
			info.waitingFor_ = taskIdCounter_;
		}
	}
}

void Launcher::EraseFinishedThreads()
{
	for (auto it = workflowThreads_.begin(); it != workflowThreads_.end(); ) {
		const auto& info = *it;
		const auto& block = pipeline_.GetBlock(info.blockIndex_);
		if (info.retVal_ != 0) {
			failedRetVal_.push_back(info.retVal_);
			it = workflowThreads_.erase(it);
		} else if (info.itemIndex_ >= block.items_.size()) {
			it = workflowThreads_.erase(it);
		} else {
			++it;
		}
	}
}

bool Launcher::GetTaskFromQueue(WorkflowTask& task)
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (taskQueue_.empty()) {
		return false;
	}
	task = taskQueue_.front();
	taskQueue_.pop_front();
	return true;
}

void Launcher::SetTaskFinished(unsigned int taskId, int retVal)
{
	std::lock_guard<std::mutex> lock(mutex_);
	finishedTasks_[taskId] = retVal;
}

void Launcher::Worker()
{
	while (!exiting_) {
		WorkflowTask task;
		if (GetTaskFromQueue(task)) {
			const auto& block = pipeline_.GetBlock(task.blockIndex_);
			const auto& item = block.items_[task.itemIndex_];
			int retVal;
			if (item.Type() == CommandItem::TYPE_PROC) {
				retVal = RunProc(item.ProcName(), item.GetProcArgs(), task.indent_);
			} else if (item.Type() == CommandItem::TYPE_SHELL) {
				retVal = RunShell(item, task.indent_);
			}
			SetTaskFinished(task.taskId_, retVal);
		} else {
			usleep(100);
		}
	}
}

int Launcher::ProcessWorkflowThreads(const ProcArgs& procArgs)
{
	if (maxJobNumber_ == 0) {
		maxJobNumber_ = std::thread::hardware_concurrency();
	}
	std::thread threads[maxJobNumber_];
	for (int i = 0; i < maxJobNumber_; ++i) {
		threads[i] = std::thread(&Launcher::Worker, std::ref(*this));
	}

	while (CheckStatus() != STATUS_EXITED) {
		Wait();
	}

	exiting_ = true;
	for (auto& thread : threads) {
		thread.join();
	}

	if (failedRetVal_.size() > 1) {
		return 1;
	} else if (failedRetVal_.size() == 1) {
		return failedRetVal_[0];
	} else {
		return 0;
	}
}

int Launcher::Run(const ProcArgs& procArgs)
{
	if (pipeline_.GetDefaultBlock().parallel_) {
		for (size_t i = 0; i < pipeline_.GetDefaultBlock().items_.size(); ++i) {
			workflowThreads_.push_back(WorkflowThread(0, i, "", procArgs)); // add every command of default block (blockIndex = 0)
		}
	} else {
		workflowThreads_.push_back(WorkflowThread(0, 0, "", procArgs)); // first command (itemIndex = 0) of default block (blockIndex = 0)
	}

	uniqueId_ = GetUniqueId();
	logDir_ = LOG_ROOT + "/" + uniqueId_;

	if (!PrepareToRun()) {
		return 1;
	}
	if (!RecordSysInfo(logDir_ + "/sysinfo")) {
		return 1;
	}
	if (!pipeline_.Save(logDir_ + "/pipeline")) {
		std::cerr << "Error: Can not write file '" << logDir_ << "/pipeline'!" << std::endl;
		return 1;
	}

	logFile_.Initialize(logDir_ + "/log");
	logFile_.WriteLine(Msg() << "[" << uniqueId_ << "] " << System::GetFullCommandLine());

	SetSigAction();

	LauncherTimer timer;

	int retVal = ProcessWorkflowThreads(procArgs);

	timer.Stop();
	if (retVal != 0) {
		logFile_.WriteLine(Msg() << "[" << uniqueId_ << "] Pipeline finished abnormally with exit value: " << retVal << "! (elapsed: " << timer.Elapse() << ")");
	} else {
		logFile_.WriteLine(Msg() << "[" << uniqueId_ << "] Pipeline finished successfully! (elapsed: " << timer.Elapse() << ")");
	}
	return retVal;
}

bool Launcher::PrepareToRun()
{
	Semaphore sem("/seqpipe");
	std::lock_guard<Semaphore> lock(sem);

	if (!System::EnsureDirectory(LOG_ROOT)) {
		return false;
	}
	if (!System::EnsureDirectory(logDir_)) {
		return false;
	}

	if (!WriteToHistoryLog()) {
		return false;
	}

	if (!CreateLastSymbolicLink()) {
		return false;
	}
	return true;
}

bool Launcher::RecordSysInfo(const std::string& filename)
{
	std::ofstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Error: Can not write to file '" << filename << "'!" << std::endl;
		return false;
	}

	file << "===== System Information =====\n"
		"System: " + System::RunShell("uname -a") +
		"\n"
		"Date: " + System::RunShell("date '+%Y-%m-%d %H:%M:%S'") +
		"Pwd : " + System::RunShell("pwd") +
		"\n"
		"CPU:\n" + System::RunShell("lscpu") +
		"\n"
		"Memory:\n" + System::RunShell("free -g") +
		"\n"
		"===== SeqPipe Version =====\n"
		"SeqPipe: " + VERSION + "\n"
		"SeqPipe Path: " + System::GetCurrentExe() + "\n"
		<< std::endl;

	file.close();
	return true;
}

bool Launcher::WriteToHistoryLog()
{
	const auto historyLog = LOG_ROOT + "/history." + System::GetHostname() + ".log";

	std::ofstream file(historyLog, std::ios::app);
	if (!file.is_open()) {
		std::cerr << "Error: Can not write to history file '" << historyLog << "'!" << std::endl;
		return false;
	}

	file << uniqueId_ << '\t' << System::GetFullCommandLine() << std::endl;
	file.close();

	return true;
}

bool Launcher::CreateLastSymbolicLink()
{
	if (System::CheckFileExists(LOG_LAST)) {
		unlink(LOG_LAST.c_str());
	}
	int retVal = symlink(uniqueId_.c_str(), LOG_LAST.c_str());
	if (retVal != 0) {
		std::cerr << "Warning: Can not create symbolic link '.seqpipe/last' to '" << uniqueId_ << "'! err: " << retVal << std::endl;
	}

	return true;
}

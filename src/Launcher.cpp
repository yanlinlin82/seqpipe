#include <iostream>
#include <fstream>
#include <regex>
#include <atomic>
#include <thread>
#include <csignal>
#include <cassert>
#include <unistd.h>
#include "Launcher.h"
#include "System.h"
#include "StringUtils.h"
#include "SeqPipe.h"
#include "Semaphore.h"
#include "TimeString.h"

Launcher::Launcher(const Pipeline& pipeline, int maxJobNumber, int verbose):
	pipeline_(pipeline), verbose_(verbose), maxJobNumber_(maxJobNumber)
{
}

std::atomic<bool> killed(false);

void MySigAction(int signum, siginfo_t* siginfo, void* ucontext)
{
	killed = true;
#if 0
	time_t now = time(NULL);
	logFile_.WriteLine(Msg() << "(0) Aborts at " + TimeString(now));
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

static std::string ExpandArgs(const std::string& s, const ProcArgs& procArgs)
{
	std::string text = s;
	std::smatch m;
	std::regex e("\\$\\{(\\w+)\\}");
	std::string res;
	while (std::regex_search(text, m, e)) { // TODO: skip variables in single-quotes string
		res += m.prefix();
		res += procArgs.Get(m[1]);
		text = m.suffix();
	}
	res += text;
	return res;
}

int Launcher::RunShell(const CommandItem& item, std::string indent, const ProcArgs& procArgs)
{
	unsigned int id = counter_.FetchId();

	const std::string name = std::to_string(id) + "." + item.Name();
	auto cmdLine = item.ShellCmd();

	cmdLine = ExpandArgs(cmdLine, procArgs);

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

void Launcher::CheckFinishedTasks()
{
	for (auto it = finishedTasks_.begin(); it != finishedTasks_.end(); ) {
		auto taskId = it->first;
		auto retVal = it->second;

		for (auto& info : workflowThreads_) {
			if (info.waitingFor_.find(taskId) != info.waitingFor_.end()) {
				const Block& block = *info.block_;
				const CommandItem& item = block.GetItems()[info.itemIndex_];
				if (item.Type() == CommandType::TYPE_PROC) {
					info.timer_.Stop();
					logFile_.WriteLine(Msg() << info.indent_ << "(" << info.id_ << ") ends at " << info.timer_.EndTime() << " (elapsed: " << info.timer_.Elapse() << ")");
				}
				info.retVal_ = retVal; // TODO: save multiple retVal
				info.waitingFor_.erase(taskId);
				if (info.waitingFor_.empty()) {
					info.finished_ = true;
				}
			}
		}
		it = finishedTasks_.erase(it);
	}

	for (auto& info : workflowThreads_) {
		if (info.finished_ && info.waitingFor_.empty()) {
			const Block& block = *info.block_;
			if (block.IsParallel()) {
				info.itemIndex_ = block.GetItems().size();
			} else {
				++info.itemIndex_;
				info.finished_ = false;
			}
		}
	}
}

void Launcher::PostBlockToThreads(const Block& block, WorkflowThread& info, std::list<WorkflowThread>& newThreads,
		const std::string& indent, const ProcArgs& procArgs)
{
	if (block.IsParallel()) {
		for (size_t i = 0; i < block.GetItems().size(); ++i) {
			++taskIdCounter_;
			newThreads.push_back(WorkflowThread(&block, i, indent, procArgs, taskIdCounter_));
			info.waitingFor_.insert(taskIdCounter_);
		}
	} else {
		++taskIdCounter_;
		newThreads.push_back(WorkflowThread(&block, 0, indent, procArgs, taskIdCounter_));
		info.waitingFor_.insert(taskIdCounter_);
	}
}

void Launcher::PostNextTasks()
{
	std::list<WorkflowThread> newThreads;

	for (auto& info : workflowThreads_) {
		const auto& block = *info.block_;
		if (info.waitingFor_.empty() && info.retVal_ == 0 && info.itemIndex_ < block.GetItems().size()) {
			const auto& item = block.GetItems()[info.itemIndex_];
			if (item.Type() == CommandType::TYPE_BLOCK) {
				PostBlockToThreads(item.GetBlock(), info, newThreads, info.indent_, info.procArgs_);
			} else if (item.Type() == CommandType::TYPE_PROC) {
				unsigned int id = counter_.FetchId();
				const auto name = std::to_string(id) + "." + item.ProcName();

				info.timer_.Start();
				info.id_ = id;
				logFile_.WriteLine(Msg() << info.indent_ << "(" << id << ") [pipeline] " << item.ProcName() << item.GetProcArgs().ToString());
				logFile_.WriteLine(Msg() << info.indent_ << "(" << id << ") starts at " << info.timer_.StartTime());

				WriteStringToFile(logDir_ + "/" + name + ".call", item.ProcName());

				++taskIdCounter_;
				PostBlockToThreads(pipeline_.GetBlock(item.ProcName()), info, newThreads, info.indent_ + "  ", item.GetProcArgs());
			} else {
				assert(item.Type() == CommandType::TYPE_SHELL);
				++taskIdCounter_;
				taskQueue_.push_back(WorkflowTask(info.block_, info.itemIndex_, info.indent_, info.procArgs_, taskIdCounter_));
				info.waitingFor_.insert(taskIdCounter_);
				Notify();
			}
		}
	}

	if (!newThreads.empty()) {
		workflowThreads_.insert(workflowThreads_.end(), newThreads.begin(), newThreads.end());
	}
}

void Launcher::EraseFinishedThreads()
{
	for (auto it = workflowThreads_.begin(); it != workflowThreads_.end(); ) {
		const auto& info = *it;
		const auto& block = *info.block_;

		if (info.retVal_ != 0) {
			failedRetVal_.push_back(info.retVal_);
			finishedTasks_[info.taskId_] = info.retVal_;
			it = workflowThreads_.erase(it);
		} else if (info.itemIndex_ >= block.GetItems().size()) {
			finishedTasks_[info.taskId_] = info.retVal_;
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
	--waitingWorker_;
	while (WaitForTask()) {
		WorkflowTask task;
		if (GetTaskFromQueue(task)) {
			const auto& block = *task.block_;
			const auto& item = block.GetItems()[task.itemIndex_];
			assert(item.Type() == CommandType::TYPE_SHELL); // it should only process 'shell cmd' in Worker()
			int retVal = RunShell(item, task.indent_, task.procArgs_);
			SetTaskFinished(task.taskId_, retVal);
		} else {
			assert(false); // should not reach here!
			std::this_thread::sleep_for(std::chrono::milliseconds(1)); // TODO: Use signal instead of sleep
		}
	}
}

bool Launcher::RecordSysInfo(const std::string& filename)
{
	std::ofstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Error: Can not write to file '" << filename << "'!" << std::endl;
		return false;
	}

	file << "system:\n"
		"  uname   : " + System::GetUName() + "\n"
		"  date    : " + TimeString(time(NULL)) + "\n"
		"  pwd     : " + System::GetCurrentDirectory() + "\n"
		"  cpu     : " + System::GetCPUInfo() + "\n"
		"  memory  : " + System::GetMemoryInfo() + "\n"
		"  user    : " + System::GetUserName() + "\n"
		"  uid     : " + std::to_string(System::GetUserId()) + "\n"
		"  login   : " + System::GetLoginName() + "\n"
		"  hostname: " + System::GetHostname() + "\n"
		"\n"
		"seqpipe:\n"
		"  version: " + VERSION + "\n"
		"  path   : " + System::GetCurrentExe() + "\n"
		<< std::endl;

	file.close();
	return true;
}

int Launcher::Run(const ProcArgs& procArgs)
{
	SetSigAction();

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

	if (pipeline_.GetDefaultBlock().IsParallel()) {
		for (size_t i = 0; i < pipeline_.GetDefaultBlock().GetItems().size(); ++i) {
			workflowThreads_.push_back(WorkflowThread(&pipeline_.GetDefaultBlock(), i, "", procArgs, ++taskIdCounter_)); // add every command of default block (blockIndex = 0)
		}
	} else {
		workflowThreads_.push_back(WorkflowThread(&pipeline_.GetDefaultBlock(), 0, "", procArgs, ++taskIdCounter_)); // first command (itemIndex = 0) of default block (blockIndex = 0)
	}

	LauncherTimer timer;

	if (maxJobNumber_ == 0) {
		maxJobNumber_ = std::thread::hardware_concurrency();
	}
	waitingWorker_ = maxJobNumber_;
	std::thread threads[maxJobNumber_];
	for (int i = 0; i < maxJobNumber_; ++i) {
		threads[i] = std::thread(&Launcher::Worker, std::ref(*this));
	}
	while (waitingWorker_ > 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	for (;;) {
		{
			std::lock_guard<std::mutex> lock(mutex_);

			CheckFinishedTasks();
			PostNextTasks();
			EraseFinishedThreads();

			if (workflowThreads_.empty()) break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	NotifyAll();
	for (auto& thread : threads) {
		thread.join();
	}

	int retVal = 0;
	if (failedRetVal_.size() > 1) {
		retVal = 1;
	} else if (failedRetVal_.size() == 1) {
		retVal = failedRetVal_[0];
	}

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
	Semaphore sem("/seqpipe." + System::GetUserName());
	std::lock_guard<Semaphore> lock(sem);

	if (!System::EnsureDirectory(LOG_ROOT)) {
		std::cerr << "Error: Can not prepare directory '" << LOG_ROOT << "'!" << std::endl;
		return false;
	}
	if (!System::EnsureDirectory(logDir_)) {
		std::cerr << "Error: Can not prepare directory '" << logDir_ << "'!" << std::endl;
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

bool Launcher::WaitForTask()
{
	std::unique_lock<std::mutex> lock(mutexWorker_);
	while (countWorker_ == 0) {
		condWorker_.wait(lock);
		if (countWorker_ == 0) {
			return false;
		}
	}
	--countWorker_;
	return true;
}

void Launcher::Notify()
{
	std::unique_lock<std::mutex> lock(mutexWorker_);
	++countWorker_;
	condWorker_.notify_one();
}

void Launcher::NotifyAll()
{
	std::unique_lock<std::mutex> lock(mutexWorker_);
	condWorker_.notify_all();
}

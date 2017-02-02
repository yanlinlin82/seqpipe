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
#include "LauncherTimer.h"
#include "TimeString.h"

WorkflowTask::WorkflowTask(size_t blockIndex, size_t itemIndex, std::string indent, ProcArgs procArgs, unsigned int taskId):
	blockIndex_(blockIndex), itemIndex_(itemIndex), indent_(indent), procArgs_(procArgs), taskId_(taskId)
{
}

WorkflowThread::WorkflowThread(size_t blockIndex, size_t itemIndex, std::string indent, ProcArgs procArgs, unsigned int taskId):
	blockIndex_(blockIndex), itemIndex_(itemIndex), indent_(indent), procArgs_(procArgs), taskId_(taskId)
{
}

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

int Launcher::RunProc(const std::string& procName, const ProcArgs& procArgs, std::string indent)
{
	unsigned int id = counter_.FetchId();

	const std::string name = std::to_string(id) + "." + procName;

	logFile_.WriteLine(Msg() << indent << "(" << id << ") [pipeline] " << procName << procArgs.ToString());
	LauncherTimer timer;
	logFile_.WriteLine(Msg() << indent << "(" << id << ") starts at " << timer.StartTime());

	WriteStringToFile(logDir_ + "/" + name + ".call", procName);

	int retVal = RunBlock(pipeline_.GetBlock(procName), procArgs, indent + "  ");

	timer.Stop();
	logFile_.WriteLine(Msg() << indent << "(" << id << ") ends at " << timer.EndTime() << " (elapsed: " << timer.Elapse() << ")");

	return retVal;
}

static std::string ExpandArgs(const std::string& s, const ProcArgs& procArgs)
{
	std::string text = s;
	std::smatch m;
	std::regex e("\\$\\{(\\w+)\\}");
	std::string res;
	while (std::regex_search(text, m, e)) {
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
	auto cmdLine = item.CmdLine();

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

int Launcher::RunBlock(const Block& block, const ProcArgs& procArgs, std::string indent)
{
	for (size_t i = 0; i < block.GetItems().size() && !killed; ++i) {
		const auto& item = block.GetItems()[i];
		int retVal = 0;
		if (item.Type() == CommandType::TYPE_PROC) {
			retVal = RunProc(item.ProcName(), item.GetProcArgs(), indent);
		} else if (item.Type() == CommandType::TYPE_SHELL) {
			retVal = RunShell(item, indent, procArgs);
		} else if (item.Type() == CommandType::TYPE_BLOCK) {
			retVal = RunBlock(pipeline_.GetBlock(item.GetBlockIndex()), procArgs, indent);
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
	for (auto it = finishedTasks_.begin(); it != finishedTasks_.end(); ) {
		auto taskId = it->first;
		auto retVal = it->second;

		for (auto& info : workflowThreads_) {
			const auto& block = pipeline_.GetBlock(info.blockIndex_);

			if (info.waitingFor_.find(taskId) != info.waitingFor_.end()) {
				info.waitingFor_.erase(taskId);
			}
			if (info.waitingFor_.empty()) {
				info.retVal_ = retVal; // TODO: save multiple retVal
				if (block.IsParallel()) {
					info.itemIndex_ = block.GetItems().size();
				} else {
					++info.itemIndex_;
				}
			}
		}
		it = finishedTasks_.erase(it);
	}
}

void Launcher::DumpWorkflowThreads() const
{
	std::cout << "==================" << std::endl;

	std::cout << "Total " << workflowThreads_.size() << " thread(s):\n";
	for (const auto& info : workflowThreads_) {
		std::cout << "  thread{" << info.taskId_ << "}: block = " << info.blockIndex_ << ", item = " << info.itemIndex_ << ", waiting for: ";
		if (info.waitingFor_.empty()) {
			std::cout << "(nothing)";
		} else {
			for (const auto& x : info.waitingFor_) {
				std::cout << x << " ";
			}
		}
		std::cout << "\n";
	}
	std::cout << "Total " << taskQueue_.size() << " task(s):\n";
	for (const auto& task : taskQueue_) {
		std::cout << "  task{" << task.taskId_ << "}: block = " << task.blockIndex_ << ", item = " << task.itemIndex_ << "\n";
	}
	std::cout << std::flush;
}

void Launcher::PostNextTasks()
{
	std::list<WorkflowThread> newThreads;

	for (auto& info : workflowThreads_) {
		const auto& block = pipeline_.GetBlock(info.blockIndex_);
		if (info.waitingFor_.empty() && info.retVal_ == 0 && info.itemIndex_ < block.GetItems().size()) {
			const auto& item = block.GetItems()[info.itemIndex_];
			if (item.Type() == CommandType::TYPE_BLOCK) {
				const auto& subBlock = pipeline_.GetBlock(item.GetBlockIndex());
				if (subBlock.IsParallel()) {
					for (size_t i = 0; i < subBlock.GetItems().size(); ++i) {
						++taskIdCounter_;
						newThreads.push_back(WorkflowThread(item.GetBlockIndex(), i, info.indent_, info.procArgs_, taskIdCounter_)); // TODO: info.procArgs_
						info.waitingFor_.insert(taskIdCounter_);
					}
				} else {
					++taskIdCounter_;
					newThreads.push_back(WorkflowThread(item.GetBlockIndex(), 0, info.indent_, info.procArgs_, taskIdCounter_)); // TODO: info.procArgs_
					info.waitingFor_.insert(taskIdCounter_);
				}
			//} else if (item.Type() == CommandType::TYPE_PROC) {
			} else {
				//assert(item.Type() == CommandType::TYPE_SHELL);
				++taskIdCounter_;
				taskQueue_.push_back(WorkflowTask(info.blockIndex_, info.itemIndex_, info.indent_, info.procArgs_, taskIdCounter_));
				info.waitingFor_.insert(taskIdCounter_);
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
		const auto& block = pipeline_.GetBlock(info.blockIndex_);

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
	while (!exiting_) {
		WorkflowTask task;
		if (GetTaskFromQueue(task)) {
			const auto& block = pipeline_.GetBlock(task.blockIndex_);
			const auto& item = block.GetItems()[task.itemIndex_];
			int retVal = 0;
			if (item.Type() == CommandType::TYPE_PROC) {
				retVal = RunProc(item.ProcName(), item.GetProcArgs(), task.indent_);
			} else if (item.Type() == CommandType::TYPE_SHELL) {
				retVal = RunShell(item, task.indent_, task.procArgs_);
			} else {
				assert(item.Type() == CommandType::TYPE_BLOCK);
				assert(false); // should not reach here
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

bool Launcher::RecordSysInfo(const std::string& filename)
{
	std::ofstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Error: Can not write to file '" << filename << "'!" << std::endl;
		return false;
	}

	file << "system:\n"
		"  uname : " + System::GetUName() + "\n"
		"  date  : " + TimeString(time(NULL)) + "\n"
		"  pwd   : " + System::GetCurrentDirectory() + "\n"
		"  cpu   : " + System::GetCPUInfo() + "\n"
		"  memory: " + System::GetMemoryInfo() + "\n"
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
	if (pipeline_.GetDefaultBlock().IsParallel()) {
		for (size_t i = 0; i < pipeline_.GetDefaultBlock().GetItems().size(); ++i) {
			workflowThreads_.push_back(WorkflowThread(0, i, "", procArgs, ++taskIdCounter_)); // add every command of default block (blockIndex = 0)
		}
	} else {
		workflowThreads_.push_back(WorkflowThread(0, 0, "", procArgs, ++taskIdCounter_)); // first command (itemIndex = 0) of default block (blockIndex = 0)
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

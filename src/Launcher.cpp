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
	pipeline_(pipeline), verbose_(verbose),
	maxJobNumber_(maxJobNumber > 0 ? maxJobNumber : std::thread::hardware_concurrency())
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

int ShellTask::Run(LogFile& logFile, const std::string& logDir, int verbose_)
{
	logFile.WriteLine(Msg() << indent_ << "(" << id_ << ") [shell] " << cmdLine_);
	LauncherTimer timer;
	logFile.WriteLine(Msg() << indent_ << "(" << id_ << ") starts at " << timer.StartTime());

	WriteStringToFile(logDir + "/" + name_ + ".cmd", cmdLine_);

	std::string fullCmdLine = "( " + cmdLine_ + " )";
	if (verbose_ > 0) {
		fullCmdLine += " 2> >(tee -a " + logDir + "/" + name_ + ".err >&2)";
		fullCmdLine += " > >(tee -a " + logDir + "/" + name_ + ".log)";
	} else {
		fullCmdLine += " 2>>" + logDir + "/" + name_ + ".err";
		fullCmdLine += " >>" + logDir + "/" + name_ + ".log";
	}
	int retVal = System::Execute(fullCmdLine.c_str());

	timer.Stop();
	logFile.WriteLine(Msg() << indent_ << "(" << id_ << ") ends at " << timer.EndTime() << " (elapsed: " << timer.Elapse() << ")");

	if (retVal != 0) {
		logFile.WriteLine(Msg() << indent_ << "(" << id_ << ") returns " << retVal);
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

void Launcher::PostTask(const Statement& block, Task& task, const std::string& indent, const ProcArgs& procArgs)
{
	if (block.IsParallel()) {
		for (size_t i = 0; i < block.GetItems().size(); ++i) {
			++taskIdCounter_;
			runningTasks_.push_back(Task(&block, i, indent, procArgs, taskIdCounter_));
			task.waitingFor_.insert(taskIdCounter_);
		}
	} else {
		++taskIdCounter_;
		runningTasks_.push_back(Task(&block, 0, indent, procArgs, taskIdCounter_));
		task.waitingFor_.insert(taskIdCounter_);
	}
}

void Task::SetEnd(LogFile& logFile, unsigned int taskId, int retVal)
{
	if (waitingFor_.find(taskId) != waitingFor_.end()) {
		if (block_) {
			const Statement& block = *block_;
			const Statement& item = block.GetItems()[itemIndex_];
			if (item.Type() == Statement::TYPE_PROC) {
				timer_.Stop();
				logFile.WriteLine(Msg() << indent_ << "(" << id_ << ") ends at " << timer_.EndTime() << " (elapsed: " << timer_.Elapse() << ")");
			}
		}
		retVal_ = retVal; // TODO: save multiple retVal
		waitingFor_.erase(taskId);
		if (waitingFor_.empty()) {
			finished_ = true;
		}
	}
}

void Launcher::Worker()
{
	for (;;) {
		ShellTask shellTask;
		{
			std::unique_lock<std::mutex> lock(shellTaskQueueMutex_);
			while (shellTaskQueue_.empty() && !runningTasks_.empty()) {
				shellTaskQueueCondVar_.wait(lock);
			}
			if (!shellTaskQueue_.empty()) {
				shellTask = shellTaskQueue_.front();
				shellTaskQueue_.pop_front();
			} else {
				break;
			}
		}
		int retVal = shellTask.Run(logFile_, logDir_, verbose_);
		{
			std::scoped_lock<std::mutex> lock(mutex_);
			finishedTasks_[shellTask.GetTaskId()] = retVal;
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

	Task rootTask;
	PostTask(pipeline_.GetDefaultStatement(), rootTask, "", procArgs);

	LauncherTimer timer;

	std::thread workers[maxJobNumber_];
	for (int i = 0; i < maxJobNumber_; ++i) {
		workers[i] = std::thread(&Launcher::Worker, std::ref(*this));
	}

	while (!rootTask.waitingFor_.empty()) {
		{
			std::scoped_lock<std::mutex> lock(mutex_);

			for (auto& task : runningTasks_) {
				const Statement& block = *task.block_;

				if (task.finished_ && task.waitingFor_.empty()) {
					if (block.IsParallel()) {
						task.itemIndex_ = block.GetItems().size();
					} else {
						++task.itemIndex_;
						task.finished_ = false;
					}
				}

				if (task.waitingFor_.empty() && task.retVal_ == 0 && task.itemIndex_ < block.GetItems().size()) {
					const auto& item = block.GetItems()[task.itemIndex_];
					if (item.Type() == Statement::TYPE_BLOCK) {
						PostTask(item, task, task.indent_, task.procArgs_);
					} else if (item.Type() == Statement::TYPE_PROC) {
						unsigned int id = counter_.FetchId();
						const auto name = std::to_string(id) + "." + item.ProcName();

						task.timer_.Start();
						task.id_ = id;
						logFile_.WriteLine(Msg() << task.indent_ << "(" << id << ") [pipeline] " << item.ProcName() << item.GetProcArgs().ToString());
						logFile_.WriteLine(Msg() << task.indent_ << "(" << id << ") starts at " << task.timer_.StartTime());

						WriteStringToFile(logDir_ + "/" + name + ".call", item.ProcName());

						++taskIdCounter_;
						PostTask(pipeline_.GetStatement(item.ProcName()), task, task.indent_ + "  ", item.GetProcArgs());
					} else {
						assert(item.Type() == Statement::TYPE_SHELL);
						++taskIdCounter_;
						{
							std::scoped_lock<std::mutex> lock(shellTaskQueueMutex_);
							unsigned int id = counter_.FetchId();
							const std::string name = std::to_string(id) + "." + item.Name();
							std::string cmdLine = item.ShellCmd();
							cmdLine = ExpandArgs(cmdLine, task.procArgs_);
							shellTaskQueue_.push_back(ShellTask(id, name, cmdLine, task.indent_, taskIdCounter_));
							shellTaskQueueCondVar_.notify_one();
						}
						task.waitingFor_.insert(taskIdCounter_);
					}
				}
			}

			for (auto it = runningTasks_.begin(); it != runningTasks_.end(); ) {
				const auto& task = *it;
				const auto& block = *task.block_;

				if (task.retVal_ != 0) {
					failedRetVal_.push_back(task.retVal_);
					finishedTasks_[task.taskId_] = task.retVal_;
					it = runningTasks_.erase(it);
				} else if (task.itemIndex_ >= block.GetItems().size()) {
					finishedTasks_[task.taskId_] = task.retVal_;
					it = runningTasks_.erase(it);
				} else {
					++it;
				}
			}

			for (auto& [taskId, retVal] : finishedTasks_) {
				rootTask.SetEnd(logFile_, taskId, retVal);
				for (auto& task : runningTasks_) {
					task.SetEnd(logFile_, taskId, retVal);
				}
			}
			finishedTasks_.clear();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	{
		std::unique_lock<std::mutex> lock(shellTaskQueueMutex_);
		shellTaskQueueCondVar_.notify_all();
	}
	for (auto& worker : workers) {
		worker.join();
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
	std::scoped_lock<Semaphore> lock(sem);

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

#include <iostream>
#include <fstream>
#include <atomic>
#include <csignal>
#include <unistd.h>
#include "Launcher.h"
#include "System.h"
#include "StringUtils.h"
#include "SeqPipe.h"
#include "Semaphore.h"
#include "LauncherTimer.h"

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

int Launcher::RunProc(const Pipeline& pipeline, const std::string& procName, std::string indent)
{
	unsigned int id = counter_.FetchId();

	const std::string name = std::to_string(id) + "." + procName;

	logFile_.WriteLine(Msg() << indent << "(" << id << ") [pipeline] " << procName);
	LauncherTimer timer;
	logFile_.WriteLine(Msg() << indent << "(" << id << ") starts at " << timer.StartTime());

	WriteStringToFile(logDir_ + "/" + name + ".pipeline", procName);

	int retVal = RunBlock(pipeline, pipeline.GetBlock(procName), indent + "  ");

	timer.Stop();
	logFile_.WriteLine(Msg() << indent << "(" << id << ") ends at " << timer.EndTime() << " (elapsed: " << timer.Elapse() << ")");

	return retVal;
}

int Launcher::RunShell(const CommandItem& item, std::string indent)
{
	unsigned int id = counter_.FetchId();

	const std::string name = std::to_string(id) + "." + StringUtils::RemoveSpecialCharacters(item.name_);
	const auto& cmdLine = item.cmdLine_;

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

int Launcher::RunBlock(const Pipeline& pipeline, const Block& block, std::string indent)
{
	for (size_t i = 0; i < block.items_.size() && !killed; ++i) {
		int retVal;
		if (pipeline.HasProcedure(block.items_[i].name_)) {
			retVal = RunProc(pipeline, block.items_[i].name_, indent);
		} else {
			retVal = RunShell(block.items_[i], indent);
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

int Launcher::Run(const Pipeline& pipeline, const std::string& procName, int verbose_)
{
	if (procName.empty()) {
		if (!pipeline.HasAnyDefaultCommand()) {
			std::cerr << "Error: Procedure name should be provided, since no any default command found in pipeline script.\n"
				"   Try 'seqpipe -l ...' to see what procedures were defined." << std::endl;
			return 1;
		}
	} else {
		if (!pipeline.HasProcedure(procName)) {
			std::cerr << "Error: Can not find procedure '" << procName << "'!" << std::endl;
			return 1;
		}
	}

	uniqueId_ = GetUniqueId();
	logDir_ = LOG_ROOT + "/" + uniqueId_;

	if (!PrepareToRun()) {
		return 1;
	}
	if (!RecordSysInfo(logDir_ + "/sysinfo")) {
		return 1;
	}
	if (!pipeline.Save(logDir_ + "/pipeline")) {
		std::cerr << "Error: Can not write file '" << logDir_ << "/pipeline'!" << std::endl;
		return 1;
	}

	logFile_.Initialize(logDir_ + "/log");
	logFile_.WriteLine(Msg() << "[" << uniqueId_ << "] " << System::GetFullCommandLine());

	SetSigAction();

	LauncherTimer timer;

	int retVal;
	if (procName.empty()) {
		retVal = RunBlock(pipeline, pipeline.GetDefaultBlock(), "");
	} else {
		retVal = RunProc(pipeline, procName, "");
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

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

static void WriteFile(const std::string& filename, const std::string& s)
{
	std::ofstream file(filename);
	file << s << std::endl;
	file.close();
}

std::atomic<bool> killed(false);

void MySigAction(int signum, siginfo_t* siginfo, void* ucontext)
{
	killed = true;
#if 0
	time_t now = time(NULL);
	logFile.WriteLine(Msg() << "(0) Aborts at " + StringUtils::TimeString(now));
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

unsigned int LauncherCounter::FetchID()
{
	std::lock_guard<std::mutex> lock(mutex_);
	return ++counter_;
}

int Launcher::RunProc(const Pipeline& pipeline, const std::string& procName, LogFile& logFile, const std::string& logDir, std::string indent, int verbose)
{
	unsigned int id = counter_.FetchID();

	const std::string name = std::to_string(id) + "." + procName;

	logFile.WriteLine(Msg() << indent << "(" << id << ") [pipeline] " << procName);
	LauncherTimer timer;
	logFile.WriteLine(Msg() << indent << "(" << id << ") starts at " << timer.StartTime());

	WriteFile(logDir + "/" + name + ".pipeline", procName);

	int retVal = RunBlock(pipeline, pipeline.GetBlock(procName), logFile, logDir, indent + "  ", verbose);

	timer.Stop();
	logFile.WriteLine(Msg() << indent << "(" << id << ") ends at " << timer.EndTime() << " (elapsed: " << timer.Elapse() << ")");

	return retVal;
}

int Launcher::RunShell(const CommandItem& item, LogFile& logFile, const std::string& logDir, std::string indent, int verbose)
{
	unsigned int id = counter_.FetchID();

	const std::string name = std::to_string(id) + "." + StringUtils::RemoveSpecialCharacters(item.name_);
	const auto& cmdLine = item.cmdLine_;

	logFile.WriteLine(Msg() << indent << "(" << id << ") [shell] " << cmdLine);
	LauncherTimer timer;
	logFile.WriteLine(Msg() << indent << "(" << id << ") starts at " << timer.StartTime());

	WriteFile(logDir + "/" + name + ".cmd", cmdLine);

	std::string fullCmdLine = "( " + cmdLine + " )";
	if (verbose > 0) {
		fullCmdLine += " 2> >(tee -a " + logDir + "/" + name + ".err >&2)";
		fullCmdLine += " > >(tee -a " + logDir + "/" + name + ".log)";
	} else {
		fullCmdLine += " 2>>" + logDir + "/" + name + ".err";
		fullCmdLine += " >>" + logDir + "/" + name + ".log";
	}
	int retVal = System::Execute(fullCmdLine.c_str());

	timer.Stop();
	logFile.WriteLine(Msg() << indent << "(" << id << ") ends at " << timer.EndTime() << " (elapsed: " << timer.Elapse() << ")");

	if (retVal != 0) {
		logFile.WriteLine(Msg() << indent << "(" << id << ") returns " << retVal);
		return retVal;
	}
	return 0;
}

int Launcher::RunBlock(const Pipeline& pipeline, const Block& block, LogFile& logFile, const std::string& logDir, std::string indent, int verbose)
{
	for (size_t i = 0; i < block.items_.size() && !killed; ++i) {
		int retVal;
		if (pipeline.HasProcedure(block.items_[i].name_)) {
			retVal = RunProc(pipeline, block.items_[i].name_, logFile, logDir, indent, verbose);
		} else {
			retVal = RunShell(block.items_[i], logFile, logDir, indent, verbose);
		}
		if (retVal != 0) {
			return retVal;
		}
	}
	return 0;
}

int Launcher::Run(const Pipeline& pipeline, const std::string& procName, int verbose)
{
	LauncherTimer timer;

	const Procedure* proc = pipeline.GetProc(procName);
	if (!proc) {
		return 1;
	}

	const auto uniqueId = System::GetUniqueId();
	const auto logDir = LOG_ROOT + "/" + uniqueId;

	if (!PrepareToRun(logDir, uniqueId)) {
		return 1;
	}
	if (!RecordSysInfo(logDir + "/sysinfo")) {
		return 1;
	}
	if (!pipeline.Save(logDir + "/pipeline")) {
		std::cerr << "Error: Can not write file '" << logDir << "/pipeline'!" << std::endl;
		return 1;
	}

	LogFile logFile(logDir + "/log");
	logFile.WriteLine(Msg() << "[" << uniqueId << "] " << System::GetFullCommandLine());

	SetSigAction();

	int retVal;
	if (procName.empty()) {
		retVal = RunBlock(pipeline, pipeline.GetBlock(""), logFile, logDir, "", verbose);
	} else {
		retVal = RunProc(pipeline, procName, logFile, logDir, "", verbose);
	}
	timer.Stop();
	if (retVal != 0) {
		logFile.WriteLine(Msg() << "[" << uniqueId << "] Pipeline finished abnormally with exit value: " << retVal << "! (elapsed: " << timer.Elapse() << ")");
	} else {
		logFile.WriteLine(Msg() << "[" << uniqueId << "] Pipeline finished successfully! (elapsed: " << timer.Elapse() << ")");
	}
	return retVal;
}

bool Launcher::PrepareToRun(const std::string& logDir, const std::string& uniqueId)
{
	Semaphore sem("/seqpipe");
	std::lock_guard<Semaphore> lock(sem);

	if (!System::EnsureDirectory(LOG_ROOT)) {
		return false;
	}
	if (!System::EnsureDirectory(logDir)) {
		return false;
	}

	if (!WriteToHistoryLog(uniqueId)) {
		return false;
	}

	if (!CreateLastSymbolicLink(uniqueId)) {
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

bool Launcher::WriteToHistoryLog(const std::string& uniqueId)
{
	const auto historyLog = LOG_ROOT + "/history." + System::GetHostname() + ".log";

	std::ofstream file(historyLog, std::ios::app);
	if (!file.is_open()) {
		std::cerr << "Error: Can not write to history file '" << historyLog << "'!" << std::endl;
		return false;
	}

	file << uniqueId << '\t' << System::GetFullCommandLine() << std::endl;
	file.close();

	return true;
}

bool Launcher::CreateLastSymbolicLink(const std::string& uniqueId)
{
	if (System::CheckFileExists(LOG_LAST)) {
		unlink(LOG_LAST.c_str());
	}
	int retVal = symlink(uniqueId.c_str(), LOG_LAST.c_str());
	if (retVal != 0) {
		std::cerr << "Warning: Can not create symbolic link '.seqpipe/last' to '" << uniqueId << "'! err: " << retVal << std::endl;
	}

	return true;
}

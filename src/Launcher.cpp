#include <iostream>
#include <fstream>
#include <atomic>
#include <regex>
#include <cassert>
#include <csignal>
#include <unistd.h>
#include "Launcher.h"
#include "System.h"
#include "StringUtils.h"
#include "SeqPipe.h"
#include "Semaphore.h"

bool Launcher::CheckIfPipeFile(const std::string& command)
{
	if (!System::CheckFileExists(command)) {
		return false;
	}
	if (System::HasExecutiveAttribute(command)) {
		return false;
	}
	if (!System::IsTextFile(command)) {
		return false;
	}
	return true;
}

bool Launcher::LoadPipeFile(const std::string& filename)
{
	assert(originPipeline_.empty());

	std::ifstream file(filename);
	if (!file) {
		return false;
	}

	std::string line;
	while (std::getline(file, line)) {
		originPipeline_.push_back(line);
		if (std::regex_match(line, std::regex("^\\s*#"))) {
			continue;
		}
		CommandItem item;
		item.name_ = StringUtils::GetFirstWord(line);
		item.cmdLine_ = line;
		commandLines_.push_back(item);
	}

	file.close();
	return true;
}

bool Launcher::WritePipeFile(const std::string& filename) const
{
	std::ofstream file(filename);
	if (!file) {
		return false;
	}

	for (const auto& s : originPipeline_) {
		file << s << std::endl;
	}

	file.close();
	return true;
}

std::string Launcher::JoinCommandLine(const std::string& cmd, const std::vector<std::string>& arguments)
{
	std::string cmdLine = cmd;
	for (const auto arg : arguments) {
		cmdLine += ' ' + System::EncodeShell(arg);
	}
	return cmdLine;
}

bool Launcher::AppendCommand(const std::string& cmd, const std::vector<std::string>& arguments)
{
	auto cmdLine = JoinCommandLine(cmd, arguments);
	originPipeline_.push_back(cmdLine);

	CommandItem item;
	item.name_ = cmd;
	item.cmdLine_ = cmdLine;
	commandLines_.push_back(item);
	return true;
}

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

int Launcher::Run(LogFile& logFile, const std::string& logDir, int verbose)
{
	struct sigaction sa = { };
	sa.sa_sigaction = MySigAction;
	sa.sa_flags = SA_SIGINFO;

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	for (size_t i = 0; i < commandLines_.size() && !killed; ++i) {
		const std::string name = std::to_string(i + 1) + "." + StringUtils::RemoveSpecialCharacters(commandLines_[i].name_);
		const auto& cmdLine = commandLines_[i].cmdLine_;

		logFile.WriteLine(Msg() << "(" << i + 1 << ") [shell] " << cmdLine);
		time_t t0 = time(NULL);
		logFile.WriteLine(Msg() << "(" << i + 1 << ") starts at " << StringUtils::TimeString(t0));

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

		time_t t = time(NULL);
		logFile.WriteLine(Msg() << "(" << i + 1 << ") ends at " << StringUtils::TimeString(t) << " (elapsed: " << StringUtils::DiffTimeString(t - t0) << ")");

		if (retVal != 0) {
			logFile.WriteLine(Msg() << "(" << i + 1 << ") returns " << retVal);
			return retVal;
		}
	}
	return 0;
}

std::vector<std::string> Launcher::GetModules() const
{
	return modules_;
}

int Launcher::Run(int verbose)
{
	const auto uniqueId = System::GetUniqueId();
	const auto logDir = LOG_ROOT + "/" + uniqueId;

	if (!PrepareToRun(logDir, uniqueId)) {
		return 1;
	}
	if (!RecordSysInfo(logDir + "/sysinfo")) {
		return 1;
	}
	if (!WritePipeFile(logDir + "/pipeline")) {
		std::cerr << "Error: Can not write file '" << logDir << "/pipeline'!" << std::endl;
		return 1;
	}

	LogFile logFile(logDir + "/log");
	logFile.WriteLine(Msg() << "[" << uniqueId << "] " << System::GetFullCommandLine());

	int retVal = Run(logFile, logDir, verbose);
	if (retVal != 0) {
		logFile.WriteLine(Msg() << "[" << uniqueId << "] Pipeline finished abnormally with exit value: " << retVal << "!");
	} else {
		logFile.WriteLine(Msg() << "[" << uniqueId << "] Pipeline finished successfully!");
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

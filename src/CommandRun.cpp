#include <iostream>
#include <fstream>
#include <vector>
#include <mutex>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include "CommandRun.h"
#include "System.h"
#include "Semaphore.h"
#include "LogFile.h"
#include "SeqPipe.h"

void CommandRun::PrintUsage()
{
	std::cout << "\n"
		"Usage:\n"
		"   seqpipe run [options] <workflow.pipe> [NAME=VALUE ...]\n"
		"   seqpipe run [options] <command> [arguments ...]\n"
		"\n"
		"Options:\n"
		"   -h         Show this help messages.\n"
		"   -v         Show verbose messages.\n"
		"   -t <int>   Max job number in parallel. default as current processor number.\n"
		"   -f         Force to re-run when output files are already latest.\n"
		"   -k         Keep temporary files.\n"
		<< std::endl;
}

std::string RemoveSpecialCharacters(const std::string& s)
{
	std::string t;
	for (size_t i = 0; i < s.size(); ++i) {
		if (s[i] == '-' || s[i] == '_' || s[i] == '+' ||
				(s[i] >= '0' && s[i] <= '9') ||
				(s[i] >= 'A' && s[i] <= 'Z') ||
				(s[i] >= 'a' && s[i] <= 'z')) {
			t += s[i];
		} else if (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n') {
			break;
		}
	}
	return t;
}

void WriteFile(const std::string& filename, const std::string& s)
{
	std::ofstream file(filename);
	file << s << std::endl;
	file.close();
}

bool CommandRun::ParseArgs(const std::list<std::string>& args)
{
	for (auto it = args.begin(); it != args.end(); ++it) {
		const auto& arg = *it;
		if ((command_.empty() || commandIsPipeFile_) && arg[0] == '-') {
			if (arg == "-h") {
				PrintUsage();
				return false;
			} else if (arg == "-v") {
				++verbose_;
			} else if (arg == "-t") {
				const auto& parameter = *(++it);
				maxJobNumber_ = std::stoi(parameter);
				if (maxJobNumber_ < 0) {
					std::cerr << "Error: Invalid number '" << parameter << "' for option '-t'!\n";
					return false;
				}
			} else if (arg == "-f") {
				forceRun_ = true;
			} else if (arg == "-k") {
				keepTemp_ = true;
			} else {
				std::cerr << "Error: Unknown option '" << arg << "'!\n";
				return false;
			}
		} else if (command_.empty()) {
			command_ = arg;
			commandIsPipeFile_ = System::CheckFileExists(command_);
		} else {
			arguments_.push_back(arg);
		}
	}
	if (command_.empty()) {
		PrintUsage();
		return false;
	}
	return true;
}

std::string TimeString(time_t t)
{
	tm tmBuf;
	localtime_r(&t, &tmBuf);
	char buffer[32] = "";
	snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
			tmBuf.tm_year + 1900, tmBuf.tm_mon + 1, tmBuf.tm_mday,
			tmBuf.tm_hour, tmBuf.tm_min, tmBuf.tm_sec);
	return buffer;
}

std::string DiffTimeString(int elapsed)
{
	std::string s;

	if (elapsed >= 86400) {
		s += std::to_string(elapsed / 86400) + "d";
		elapsed %= 86400;
	}
	if (elapsed >= 3600) {
		s += (s.empty() ? "" : " ") + std::to_string(elapsed / 3600) + "h";
		elapsed %= 3600;
	}
	if (elapsed >= 60) {
		s += (s.empty() ? "" : " ") + std::to_string(elapsed / 60) + "m";
		elapsed %= 60;
	}
	if (s.empty() || elapsed > 0) {
		s += (s.empty() ? "" : " ") + std::to_string(elapsed) + "s";
	}
	return s;
}

std::string JoinCommandLine(const std::string& cmd, const std::vector<std::string>& arguments)
{
	std::string cmdLine = cmd;
	for (const auto arg : arguments) {
		cmdLine += ' ' + System::EncodeShell(arg);
	}
	return cmdLine;
}

int CommandRun::Run(const std::list<std::string>& args)
{
	if (!ParseArgs(args)) {
		return 1;
	}

	if (verbose_ > 0) {
		if (commandIsPipeFile_) {
			std::cerr << "Run pipe file: '" << command_ << "' with:\n";
		} else {
			std::cerr << "Run command: '" << command_ << "' with:\n";
		}
		for (const auto arg : arguments_) {
			std::cerr << "  [" << arg << "]" << std::endl;
		}
	}

	const auto uniqueId = System::GetUniqueId();
	const auto logDir = LOG_ROOT + "/" + uniqueId;

	if (!System::CheckDirectoryExists(LOG_ROOT)) {
		if (verbose_ > 0) {
			std::cerr << "Create directory: '" << LOG_ROOT << "'" << std::endl;
		}
		System::CreateDirectory(LOG_ROOT);
	}
	if (!System::CheckDirectoryExists(logDir)) {
		if (verbose_ > 0) {
			std::cerr << "Create directory: '" << logDir << "'" << std::endl;
		}
		System::CreateDirectory(logDir);
	}

	WriteToHistoryLog(uniqueId);
	CreateLastSymbolicLink(uniqueId);

	log_ = logDir + "/log";
	counter_ = 0;

	LogFile logFile(log_);
	logFile.WriteLine(Msg() << "[" << uniqueId << "] " << System::GetFullCommandLine());

	if (commandIsPipeFile_) {
		std::cerr << "Error: Unimplemented!\n";
		return 1;
	}

	++counter_;
	const auto cmdLine = JoinCommandLine(command_, arguments_);

	logFile.WriteLine(Msg() << "(" << counter_ << ") [shell] " << cmdLine);
	time_t t0 = time(NULL);
	logFile.WriteLine(Msg() << "(" << counter_ << ") starts at " << TimeString(t0));

	std::string name = std::to_string(counter_) + "." + RemoveSpecialCharacters(command_);

	WriteFile(logDir + "/" + name + ".cmd", cmdLine);

	std::string fullCmdLine = "( " + cmdLine + " )";
	if (verbose_ > 0) {
		fullCmdLine += " 2> >(tee -a " + logDir + "/" + name + ".err >&2)";
		fullCmdLine += " > >(tee -a " + logDir + "/" + name + ".log)";
	} else {
		fullCmdLine += " 2>>" + logDir + "/" + name + ".err";
		fullCmdLine += " >>" + logDir + "/" + name + ".log";
	}
	int retVal = System::Execute(fullCmdLine.c_str());

	time_t t = time(NULL);
	logFile.WriteLine(Msg() << "(" << counter_ << ") ends at " << TimeString(t) << " (elapsed: " << DiffTimeString(t - t0) << ")");

	if (retVal != 0) {
		logFile.WriteLine(Msg() << "[" << uniqueId << "] Pipeline finished abnormally with exit value: " << retVal << "!");
	} else {
		logFile.WriteLine(Msg() << "[" << uniqueId << "] Pipeline finished successfully!");
	}
	return retVal;
}

bool CommandRun::WriteToHistoryLog(const std::string& uniqueId)
{
	const auto historyLog = LOG_ROOT + "/history." + System::GetHostname() + ".log";

	Semaphore sem("/seqpipe");
	std::lock_guard<Semaphore> lock(sem);

	std::ofstream file(historyLog, std::ios::app);
	if (!file.is_open()) {
		std::cerr << "Error: Can not write to history file '" << historyLog << "'!" << std::endl;
		return false;
	}

	file << uniqueId << '\t' << System::GetFullCommandLine() << std::endl;
	file.close();

	return true;
}

bool CommandRun::CreateLastSymbolicLink(const std::string& uniqueId)
{
	Semaphore sem("/seqpipe");
	std::lock_guard<Semaphore> lock(sem);

	if (System::CheckFileExists(LOG_LAST)) {
		unlink(LOG_LAST.c_str());
	}
	int retVal = symlink(uniqueId.c_str(), LOG_LAST.c_str());
	if (retVal != 0) {
		std::cerr << "Warning: Can not create symbolic link '.seqpipe/last' to '" << uniqueId << "'! err: " << retVal << std::endl;
	}

	return true;
}

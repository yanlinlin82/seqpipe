#include <iostream>
#include <fstream>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include "CommandRun.h"
#include "System.h"
#include "Semaphore.h"
#include "LogFile.h"
#include "SeqPipe.h"
#include "StringUtils.h"

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

bool CommandRun::ParseArgs(const std::list<std::string>& args)
{
	std::string cmd;
	bool cmdIsPipeFile = false;
	std::vector<std::string> arguments;

	for (auto it = args.begin(); it != args.end(); ++it) {
		const auto& arg = *it;
		if ((cmd.empty() || cmdIsPipeFile) && arg[0] == '-') {
			if (arg == "-h") {
				PrintUsage();
				return false;
			} else if (arg == "-v") {
				++verbose_;
			} else if (arg == "-t") {
				const auto& parameter = *(++it);
				maxJobNumber_ = std::stoi(parameter);
				if (maxJobNumber_ < 0) {
					std::cerr << "Error: Invalid number '" << parameter << "' for option '-t'!" << std::endl;
					return false;
				}
			} else if (arg == "-f") {
				forceRun_ = true;
			} else if (arg == "-k") {
				keepTemp_ = true;
			} else {
				std::cerr << "Error: Unknown option '" << arg << "'!" << std::endl;
				return false;
			}
		} else if (cmd.empty()) {
			cmd = arg;
			if (!launcher_.CheckIfPipeFile(cmd)) {
				cmdIsPipeFile = false;
			} else {
				if (!launcher_.LoadPipeFile(cmd)) {
					std::cerr << "Error: Failed to load pipe file '" << cmd << "'!" << std::endl;
					return false;
				}
				cmdIsPipeFile = true;
			}
		} else {
			arguments.push_back(arg);
		}
	}
	if (cmd.empty()) {
		PrintUsage();
		return false;
	}
	if (!cmdIsPipeFile) {
		launcher_.AppendCommand(cmd, arguments);
	}
	return true;
}

bool CommandRun::PrepareToRun(const std::string& logDir, const std::string& uniqueId)
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

int CommandRun::Run(const std::list<std::string>& args)
{
	if (!ParseArgs(args)) {
		return 1;
	}

	const auto uniqueId = System::GetUniqueId();
	const auto logDir = LOG_ROOT + "/" + uniqueId;

	if (!PrepareToRun(logDir, uniqueId)) {
		return 1;
	}

	LogFile logFile(logDir + "/log");
	logFile.WriteLine(Msg() << "[" << uniqueId << "] " << System::GetFullCommandLine());

	int retVal = launcher_.Run(logFile, logDir, verbose_);
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
	if (System::CheckFileExists(LOG_LAST)) {
		unlink(LOG_LAST.c_str());
	}
	int retVal = symlink(uniqueId.c_str(), LOG_LAST.c_str());
	if (retVal != 0) {
		std::cerr << "Warning: Can not create symbolic link '.seqpipe/last' to '" << uniqueId << "'! err: " << retVal << std::endl;
	}

	return true;
}

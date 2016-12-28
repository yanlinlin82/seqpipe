#include <iostream>
#include <fstream>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include "Launcher.h"
#include "System.h"

static const std::string LOG_ROOT = ".seqpipe";

void Launcher::PrintUsage()
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

bool Launcher::ParseArgs(const std::list<std::string>& args)
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

int Launcher::Run(const std::list<std::string>& args)
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

	if (commandIsPipeFile_) {
		std::cerr << "Error: Unimplemented!\n";
		return 1;
	}

	return RunCommand(command_, arguments_);
}

int Launcher::RunCommand(const std::string& cmd, const std::vector<std::string>& arguments)
{
	std::string cmdLine = command_;
	for (const auto arg : arguments_) {
		cmdLine += ' ' + System::EncodeShell(arg);
	}
	return system(cmdLine.c_str());
}

bool Launcher::WriteToHistoryLog(const std::string& uniqueId)
{
	// TODO: lock for 'history.log'
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
	// TODO: lock for 'last'
	const auto lastLink = LOG_ROOT + "/last";

	if (System::CheckFileExists(lastLink)) {
		unlink(lastLink.c_str());
	}
	int retVal = symlink(uniqueId.c_str(), lastLink.c_str());
	if (retVal != 0) {
		std::cerr << "Warning: Can not create symbolic link '.seqpipe/last' to '" << uniqueId << "'! err: " << retVal << std::endl;
	}

	return true;
}

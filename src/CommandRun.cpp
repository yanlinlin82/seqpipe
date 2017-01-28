#include <iostream>
#include <fstream>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include "CommandRun.h"
#include "System.h"
#include "LogFile.h"
#include "SeqPipe.h"
#include "Launcher.h"
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
	bool loaded = false;

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
			} else if (arg == "-") {
				if (cmd.empty()) {
					if (isatty(fileno(stdin))) {
						std::cerr << "Error: Failed to load pipe file from stdin!" << std::endl;
						return false;
					}
					cmd = "/dev/stdin";
					if (!pipeline_.Load(cmd)) {
						std::cerr << "Error: Failed to load pipe file from stdin!" << std::endl;
						return false;
					}
					loaded = true;
				} else {
					std::cerr << "Error: Unexpected option '" << arg << "'!" << std::endl;
					return false;
				}
			} else {
				std::cerr << "Error: Unknown option '" << arg << "'!" << std::endl;
				return false;
			}
		} else if (cmd.empty()) {
			cmd = arg;
			if (!Pipeline::CheckIfPipeFile(cmd)) {
				cmdIsPipeFile = false;
			} else {
				if (!pipeline_.Load(cmd)) {
					std::cerr << "Error: Failed to load pipe file '" << cmd << "'!" << std::endl;
					return false;
				}
				cmdIsPipeFile = true;
				loaded = true;
			}
		} else {
			arguments.push_back(arg);
		}
	}
	if (isatty(fileno(stdin))) {
		if (cmd.empty()) {
			PrintUsage();
			return false;
		}
		if (!cmdIsPipeFile) {
			pipeline_.AppendCommand(cmd, arguments);
		}
	} else if (!loaded) {
		if (!pipeline_.Load("/dev/stdin")) {
			std::cerr << "Error: Failed to load pipe from stdin!" << std::endl;
			return false;
		}
	}
	return true;
}

int CommandRun::Run(const std::list<std::string>& args)
{
	if (!ParseArgs(args)) {
		return 1;
	}

	Launcher launcher;
	return launcher.Run(pipeline_, "", verbose_);
}

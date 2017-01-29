#include <iostream>
#include <unistd.h>
#include "CommandParallel.h"
#include "Launcher.h"

void CommandParallel::PrintUsage()
{
	std::cout << "\n"
		"Usage:\n"
		"   seqpipe parallel [options] <commands.txt> [KEY=VALUE ...]\n"
		"\n"
		"Options:\n"
		"   -h         Show help messages.\n"
		"   -v         Show verbose messages.\n"
		"   -t <int>   Max job number in parallel. default as current processor number.\n"
		<< std::endl;
}

bool CommandParallel::LoadCommandList(const std::string& filename)
{
	std::ifstream file(filename);
	if (!file.is_open()) {
		return false;
	}
	std::string line;
	while (std::getline(file, line)) {
		commandList_.push_back(line);
	}
	file.close();
	return true;
}

bool CommandParallel::ParseArgs(const std::vector<std::string>& args)
{
	bool loaded = false;

	for (auto it = args.begin(); it != args.end(); ++it) {
		const auto& arg = *it;
		if (arg[0] == '-') {
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
			} else if (arg == "-") {
				if (commandFilename_.empty()) {
					if (isatty(fileno(stdin))) {
						std::cerr << "Error: Failed to load command list from stdin!" << std::endl;
						return false;
					}
					commandFilename_ = "/dev/stdin";
					if (!LoadCommandList(commandFilename_)) {
						std::cerr << "Error: Failed to load command list from stdin!" << std::endl;
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
		}
	}

	if (isatty(fileno(stdin))) {
		if (commandFilename_.empty()) {
			PrintUsage();
			return false;
		}
	} else if (!loaded) {
		if (!LoadCommandList("/dev/stdin")) {
			std::cerr << "Error: Failed to load pipe from stdin!" << std::endl;
			return false;
		}
	}

	pipeline_.SetDefaultBlock(commandList_, true);
	return true;
}

int CommandParallel::Run(const std::vector<std::string>& args)
{
	if (!ParseArgs(args)) {
		return 1;
	}

	Launcher launcher;
	return launcher.Run(pipeline_, verbose_);
}

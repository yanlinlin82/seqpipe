#include <iostream>
#include <regex>
#include "CommandRunDeprecated.h"

void CommandRunDeprecated::PrintUsage()
{
	std::cout << "\n"
		"Deprecated Usage:\n"
		"   seqpipe [options] <procedure> [NAME=VALUE ...]\n"
		"   seqpipe [options] -e <cmd> [-e <cmd> ...] [NAME=VALUE ...]\n"
		"   seqpipe [options] -E <cmd> [-E <cmd> ...] [NAME=VALUE ...]\n"
		"\n"
		"Options:\n"
		"   -h / -H              Show help messages.\n"
		"   -v                   Show verbose messages.\n"
		"   -e                   Run shell command directly, without writing .pipe file.\n"
		"   -E                   Same as '-e', except running in parallel.\n"
		"   -l / -L [<pattern>]  List current available procedures.\n"
		"   -m <file>            Load procedure module file, this option can be used many times.\n"
		"   -t <int>             Max thread number in parallel. default: $max_thread_number\n"
		"   -f                   Force to re-run when output files are already latest.\n"
		"   -k                   Keep temporary files.\n"
		"\n"
		"Type 'seqpipe' only to see help messages for latest usage.\n"
		<< std::endl;
}

bool CommandRunDeprecated::ParseArgs(const std::list<std::string>& args)
{
	std::vector<std::string> cmdList;
	bool parallel = false;

	for (auto it = args.begin(); it != args.end(); ++it) {
		const auto& arg = *it;
		if (arg[0] == '-') {
			if (arg == "-h") {
				helpMode_ = 1;
			} else if (arg == "-H") {
				helpMode_ = 2;
			} else if (arg == "-v") {
				++verbose_;
			} else if (arg == "-e" || arg == "-E") {
				const auto& cmd = *(++it);
				bool isParallelOption = (arg == "-E");
				if (!proc_.empty()) {
					std::cerr << "Error: Unexpected option '" << arg << "' after procedure name!" << std::endl;
					return false;
				}
				if (!cmdList.empty() && parallel != isParallelOption) {
					std::cerr << "Error: Can not use both '-e' and '-E'!" << std::endl;
					return false;
				}
				cmdList.push_back(cmd);
				parallel = isParallelOption;
			} else if (arg == "-l") {
				listMode_ = 1;
			} else if (arg == "-L") {
				listMode_ = 2;
			} else if (arg == "-m") {
				const auto& moduleFile = *(++it);
				if (!launcher_.LoadPipeFile(moduleFile)) {
					std::cerr << "Error: Failed to load pipe file '" << moduleFile << "'!" << std::endl;
					return false;
				}
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
			}
		} else if (proc_.empty() && cmdList.empty()) {
			proc_ = arg;
		} else {
			std::smatch sm;
			if (!std::regex_match(arg, sm, std::regex("^(\\w+)=(.*)$"))) {
				std::cerr << "Error: Invalid option '" << arg << "'! Expecting format 'NAME=VALUE'" << std::endl;
			}
			const auto& name = sm[1];
			const auto& value = sm[2];
		}
	}
	return true;
}

void CommandRunDeprecated::ListModules()
{
	const auto modules = launcher_.GetModules();
	std::cout << std::endl;
	for (const auto& m : modules) {
		std::cout << "   " << m << std::endl;
	}
	std::cout << std::endl;
}

int CommandRunDeprecated::Run(const std::list<std::string>& args)
{
	if (!ParseArgs(args)) {
		return 1;
	}

	if (helpMode_ > 0) {
		PrintUsage();
		return 1;
	}
	if (listMode_ > 0) {
		ListModules();
		return 0;
	}

	return 0;
}

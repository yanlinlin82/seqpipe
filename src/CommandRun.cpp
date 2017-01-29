#include <iostream>
#include <fstream>
#include <vector>
#include <regex>
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
		"   seqpipe run   [options] <cmd> [args ...]\n"
		"   seqpipe [run] [options] <workflow.pipe> [procedure] [KEY=VALUE ...]\n"
		"   seqpipe [run] [options] -e \"<cmd>\" [-e \"<cmd>\" ...] [KEY=VALUE ...]\n"
		"   seqpipe [run] [options] -E \"<cmd>\" [-E \"<cmd>\" ...] [KEY=VALUE ...]\n"
		"\n"
		"Options:\n"
		"   -h / -H             Show help messages.\n"
		"   -v                  Show verbose messages.\n"
		"   -e                  Run shell command directly, without writing .pipe file.\n"
		"   -E                  Same as '-e', except running in parallel.\n"
		"   -m <file>           Load procedure module file, this option can be used many times.\n"
		"   -l / -L [pattern]   List current available procedures.\n"
		"   -t <int>            Max job number in parallel. default as current processor number.\n"
		"   -f                  Force to re-run when output files are already latest.\n"
		"   -k                  Keep temporary files.\n"
		<< std::endl;
}

bool CommandRun::ParseArgs(const std::list<std::string>& args)
{
	std::vector<std::string> cmdList; // for '-e' or '-E'
	bool parallel = false;
	std::vector<std::string> moduleFilenames; // for '-m'
	std::string cmd;
	bool isShellCmd = false;
	std::vector<std::string> shellArgs;
	std::map<std::string, std::string> procArgs;

	for (auto it = args.begin(); it != args.end(); ++it) {
		const auto& arg = *it;
		if (arg[0] == '-' && arg != "-") {
			if (arg == "-h") {
				helpMode_ = 1;
			} else if (arg == "-H") {
				helpMode_ = 2;
			} else if (arg == "-v") {
				++verbose_;
			} else if (arg == "-e" || arg == "-E") {
				if (!cmd.empty()) {
					std::cerr << "Error: Can not use '" << arg << "' after <workflow.pipe>!" << std::endl;
					return false;
				}
				if (cmdList.empty()) {
					parallel = (arg == "-E");
				} else if (parallel != (arg == "-E")) {
					std::cerr << "Error: Can not use both '-e' and '-E'!" << std::endl;
					return false;
				}
				const auto& cmd = *(++it);
				cmdList.push_back(cmd);
			} else if (arg == "-l") {
				listMode_ = 1;
			} else if (arg == "-L") {
				listMode_ = 2;
			} else if (arg == "-m") {
				const auto& filename = *(++it);
				moduleFilenames.push_back(filename);
			} else if (arg == "-t") {
				const auto& value = *(++it);
				maxJobNumber_ = std::stoi(value);
				if (maxJobNumber_ < 0) {
					std::cerr << "Error: Invalid number '" << value << "' for option '-t'!" << std::endl;
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
		} else if (cmd.empty() && cmdList.empty() && moduleFilenames.empty()) {
			cmd = arg;
			isShellCmd = System::IsShellCmd(cmd);
			if (isShellCmd) {
				shellArgs = std::vector<std::string>(++it, args.end());
				break;
			}
		} else if (procedureName_.empty() && procArgs.empty() && std::regex_match(arg, std::regex("\\w+"))) {
			procedureName_ = arg;
		} else {
			std::smatch sm;
			if (!std::regex_match(arg, sm, std::regex("(\\w+)=(.*)"))) {
				std::cerr << "Error: Invalid option '" << arg << "'! Expecting format 'KEY=VALUE'" << std::endl;
				return false;
			}
			const auto& name = sm[1];
			const auto& value = sm[2];
			procArgs[name] = value;
		}
	}

	if (cmd.empty() && cmdList.empty() && moduleFilenames.empty()) {
		PrintUsage();
		return false;
	}

	if (!cmdList.empty()) {
		if (!pipeline_.SetDefaultBlock(cmdList, parallel)) {
			return false;
		}
	} else if (isShellCmd) {
		if (!pipeline_.SetDefaultBlock(cmd, shellArgs)) {
			return false;
		}
	} else if (!cmd.empty()) {
		if (!pipeline_.Load(cmd)) {
			return false;
		}
	} else {
		for (auto filename : moduleFilenames) {
			if (!pipeline_.Load(filename)) {
				return false;
			}
		}
	}
	return true;
}

void CommandRun::ListModules()
{
	std::cout << "\nCurrent available user-defined procedures:\n";
	for (const auto& name : pipeline_.GetProcNameList()) {
		std::cout << "   " << name << "\n";
	}
	std::cout << std::endl;
}

int CommandRun::Run(const std::list<std::string>& args)
{
	if (!ParseArgs(args)) {
		return 1;
	}

	if (listMode_ > 0) {
		ListModules();
		return 0;
	}
	if (helpMode_ > 0) {
		PrintUsage();
		return 1;
	}

	Launcher launcher;
	return launcher.Run(pipeline_, procedureName_, verbose_);
}

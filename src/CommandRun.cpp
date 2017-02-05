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
#include "CommandLineParser.h"

void CommandRun::PrintUsage()
{
	std::cout << "\n"
		"Usage:\n"
		"   seqpipe [run] [options] <workflow.pipe> [procedure] [KEY=VALUE ...]\n"
		"   seqpipe [run] [options] <cmd> [args ...]\n"
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

bool CommandRun::ParseArgs(const std::vector<std::string>& args)
{
	std::vector<std::string> cmdList; // for '-e' or '-E'
	bool parallel = false;
	std::vector<std::string> moduleFilenames; // for '-m'
	std::string cmd;
	bool isShellCmd = false;
	std::vector<std::string> shellArgs;
	std::string procName;

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
				CommandLineParser parser;
				if (!parser.Parse(cmd)) {
					std::cerr << "Error: Invalid bash command string after '" << arg << "':\n"
						"   " << cmd << "\n"
						"   " << parser.ErrorWithLeadingSpaces() << std::endl;
					return false;
				}
				cmdList.push_back(cmd);
			} else if (arg == "-l" || arg == "-L") {
				listMode_ = (arg == "-l" ? 1 : 2);
				if (it + 1 != args.end() && (*(it + 1))[0] != '-') {
					pattern_ = *(++it);
				}
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
		} else if (procName.empty() && procArgs_.IsEmpty() && std::regex_match(arg, std::regex("\\w+"))) {
			procName = arg;
		} else {
			std::smatch sm;
			if (!std::regex_match(arg, sm, std::regex("(\\w+)=(.*)"))) {
				std::cerr << "Error: Invalid option '" << arg << "'! Expecting format 'KEY=VALUE'" << std::endl;
				return false;
			}
			const auto& key = sm[1];
			const auto& value = sm[2];
			if (procArgs_.Has(key)) {
				std::cerr << "Error: Duplicated option '" << key << "'!" << std::endl;
				return false;
			}
			procArgs_.Add(key, value);
		}
	}

	if (cmd.empty() && cmdList.empty() && moduleFilenames.empty() && !listMode_) {
		PrintUsage();
		return false;
	}

	if (!cmdList.empty()) {
		pipeline_.SetDefaultBlock(parallel, cmdList);
	} else if (isShellCmd) {
		for (const auto& arg : shellArgs) {
			cmd += " " + StringUtils::ShellQuote(arg, false);
		}
		cmdList.push_back(cmd);
		pipeline_.SetDefaultBlock(false, cmdList);
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

	if (!procName.empty()) {
		if (!pipeline_.HasProcedure(procName)) {
			std::cerr << "Error: Can not find procedure '" << procName << "'!" << std::endl;
			return 1;
		}
		pipeline_.ClearDefaultBlock();
		pipeline_.SetDefaultBlock(procName, procArgs_);
		procArgs_.Clear();
	}

	pipeline_.FinalCheckAfterLoad();
	return true;
}

void CommandRun::ListModules()
{
	std::cout << "\nCurrent available procedures";
	if (!pattern_.empty()) {
		std::cout << " (search for '" << pattern_ << "')";
	}
	std::cout << ":\n";

	for (const auto& name : pipeline_.GetProcNameList(pattern_)) {
		std::cout << "   " << name << "\n";
	}
	std::cout << std::endl;
}

int CommandRun::Run(const std::vector<std::string>& args)
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

	if (!pipeline_.HasAnyDefaultCommand()) {
		std::cerr << "Error: Procedure name should be provided, since no any default command found in pipeline script.\n"
			"   Try 'seqpipe -l ...' to see what procedures were defined." << std::endl;
		return 1;
	}

	Launcher launcher(pipeline_, maxJobNumber_, verbose_);
	return launcher.Run(procArgs_);
}

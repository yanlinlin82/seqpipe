#include <iostream>
#include <regex>
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

bool CommandParallel::LoadCmdLineList(const std::string& filename, std::vector<std::string>& cmdList)
{
	const auto path = (filename == "-" ? "/dev/stdin" : filename);
	std::ifstream file(path);
	if (!file.is_open()) {
		std::cerr << "Error: Can not load commands from file '" << path << "'!" << std::endl;
		return false;
	}
	std::string line;
	size_t lineNo = 0;
	while (std::getline(file, line)) {
		++lineNo;
		CommandLineParser parser;
		if (!parser.Parse(line)) {
			std::cerr << "Error: Invalid bash command string at line " << lineNo << " of '" << path << "':\n"
				<< "   " << line << "\n"
				<< "   " << parser.ErrorWithLeadingSpaces() << std::endl;
			return false;
		}
		cmdList.push_back(line);
	}
	file.close();
	return true;
}

bool CommandParallel::ParseArgs(const std::vector<std::string>& args)
{
	std::string cmdListFilename;

	for (auto it = args.begin(); it != args.end(); ++it) {
		const auto& arg = *it;
		if (arg[0] == '-' && arg != "-") {
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
			} else {
				std::cerr << "Error: Unknown option '" << arg << "'!" << std::endl;
				return false;
			}
		} else if (cmdListFilename.empty()) {
			cmdListFilename = arg;
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

	if (cmdListFilename.empty()) {
		PrintUsage();
		return false;
	}

	std::vector<std::string> cmdList;
	if (!LoadCmdLineList(cmdListFilename, cmdList)) {
		return false;
	}

	pipeline_.SetDefaultBlock(true, cmdList);
	return true;
}

int CommandParallel::Run(const std::vector<std::string>& args)
{
	if (!ParseArgs(args)) {
		return 1;
	}

	Launcher launcher(pipeline_, maxJobNumber_, verbose_);
	return launcher.Run(procArgs_);
}

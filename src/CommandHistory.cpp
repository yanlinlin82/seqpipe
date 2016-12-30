#include <iostream>
#include <cassert>
#include "CommandHistory.h"

void CommandHistory::PrintUsage()
{
	std::cout << "\n"
		"Usage:\n"
		"   seqpipe history [list]\n"
		"   seqpipe history show [id_or_order]\n"
		"   seqpipe history remove [id_or_order]\n"
		"\n"
		"Options:\n"
		"   -h   Show this help messages.\n"
		"   -v   Show verbose messages.\n"
		<< std::endl;
}

bool CommandHistory::ParseArgs(const std::list<std::string>& args)
{
	for (auto it = args.begin(); it != args.end(); ++it) {
		const auto& arg = *it;
		if (arg[0] == '-') {
			if (arg == "-h") {
				PrintUsage();
				return false;
			} else if (arg == "-v") {
				++verbose_;
			} else {
				std::cerr << "Error: Unknown option '" << arg << "'!\n";
				return false;
			}
		} else if (command_.empty()) {
			if (arg != "list" && arg != "show" && arg != "remove") {
				std::cerr << "Error: Unknown command '" << arg << "' for 'seqpipe history'!\n";
				return false;
			}
			command_ = arg;
		} else {
			if (command_ == "list") {
				std::cerr << "Error: Unexpected paramter '" << arg << "'!\n";
				return false;
			} else if (idOrOrder_.empty()) {
				idOrOrder_ = arg;
			} else {
				std::cerr << "Error: Unexpected parameter '" << arg << "'!\n";
				return false;
			}
		}
	}
	if (command_.empty()) {
		command_ = "list";
	}
	return true;
}

int CommandHistory::Run(const std::list<std::string>& args)
{
	if (!ParseArgs(args)) {
		return 1;
	}

	if (command_ == "list") {
		return ListHistory();
	} else if (command_ == "show") {
		return ShowHistory();
	} else {
		assert(command_ == "remove");
		return RemoveHistory();
	}
}

int CommandHistory::ListHistory()
{
	return system("cat .seqpipe/history.*.log | sort | less -X");
}

int CommandHistory::ShowHistory()
{
	return system(("cat .seqpipe/" + idOrOrder_ + "/log | less -X").c_str());
}

int CommandHistory::RemoveHistory()
{
	std::cerr << "Not implemented!" << std::endl;
	return 1;
}

#include <iostream>
#include "History.h"

void History::PrintUsage()
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

bool History::ParseArgs(const std::list<std::string>& args)
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
		}
	}
	return true;
}

int History::Run(const std::list<std::string>& args)
{
	if (!ParseArgs(args)) {
		return 1;
	}

	// TODO: Implemetation
	return 0;
}

#include <iostream>
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
		"   -D                   Disable to load default pipelines.\n"
		"   -t <int>             Max thread number in parallel. default: $max_thread_number\n"
		"   -s <shell>           Send commands to another shell, default: $shell\n"
		"   -f                   Force to re-run when output files are already latest.\n"
		"   -k                   Keep temporary files.\n"
		"   -R                   Show the raw procedure declaration.\n"
		"   -T                   Test mode, show commands rather than execute them.\n"
		"\n"
		"Type 'seqpipe' only to see help messages for latest usage.\n"
		<< std::endl;
}

bool CommandRunDeprecated::ParseArgs(const std::list<std::string>& args)
{
	for (auto it = args.begin(); it != args.end(); ++it) {
		const auto& arg = *it;
		if (arg == "-h" || arg == "-H") {
			helpMode_ += (arg == "-h" ? 1 : 2);
		} else if (arg == "-v") {
			++verbose_;
		} else if (arg == "-e") {
		} else if (arg == "-E") {
		} else if (arg == "-l" || arg == "-L") {
		} else if (arg == "-m") {
		} else if (arg == "-D") {
		} else if (arg == "-t") {
		} else if (arg == "-s") {
		} else if (arg == "-f") {
		} else if (arg == "-k") {
		} else if (arg == "-R") {
		} else if (arg == "-T") {
		}
	}

	if (helpMode_ > 0) {
		PrintUsage();
		return false;
	}
	return true;
}

int CommandRunDeprecated::Run(const std::list<std::string>& args)
{
	if (!ParseArgs(args)) {
		return 1;
	}

	return 0;
}

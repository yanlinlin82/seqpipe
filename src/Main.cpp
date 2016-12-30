#include <iostream>
#include <string>
#include <list>
#include "CommandRun.h"
#include "CommandHistory.h"

static const std::string VERSION = "0.5.0 (" GITVER ")";

static void PrintUsage()
{
	std::cout << "\n"
		"SeqPipe: A framework for SEQuencing data analysis PIPElines\n"
		"Version: " << VERSION << "\n"
		"Author : Linlin Yan (yanlinlin82<at>gmail.com)\n"
		"Website: https://github.com/yanlinlin82/seqpipe\n"
		"\n"
		"Usage:\n"
		"   seqpipe run [options] <workflow.pipe> [NAME=VALUE ...]\n"
		"   seqpipe run [options] <command> [arguments ...]\n"
		"   seqpipe history [options]\n"
		"   seqpipe version\n"
		"\n"
		"Options:\n"
		"   -h   Show help messages.\n"
		"   -v   Show verbose messages.\n"
		<< std::endl;
}

int main(int argc, const char** argv)
{
	std::list<std::string> args(argv + 1, argv + argc);

	if (args.empty()) {
		PrintUsage();
		return 1;
	}

	const auto cmd = args.front();
	args.pop_front();

	if (cmd == "run") {
		CommandRun cmd;
		return cmd.Run(args);

	} else if (cmd == "history") {
		CommandHistory cmd;
		return cmd.Run(args);

	} else if (cmd == "version") {
		std::cout << VERSION << std::endl;
		return 0;
	}

	std::cerr << "Error: Unknown command '" << cmd << "'" << std::endl;
	return 1;
}

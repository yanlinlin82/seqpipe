#include <iostream>
#include <string>
#include <vector>
#include "CommandRun.h"
#include "CommandParallel.h"
#include "CommandLog.h"
#include "CommandHelp.h"
#include "SeqPipe.h"

static void PrintUsage()
{
	std::cout << "\n"
		"SeqPipe: a framework for SEQuencing data analysis PIPElines\n"
		"Version: " << VERSION << "\n"
		"Author : Linlin Yan (yanlinlin82<at>gmail.com)\n"
		"Website: https://github.com/yanlinlin82/seqpipe/tree/cpp-v0.5\n"
		"\n"
		"Usage:\n"
		"   seqpipe [run]    [options] <workflow.pipe> [procedure] [KEY=VALUE ...]\n"
		"   seqpipe [run]    [options] <cmd> [args ...]\n"
		"   seqpipe parallel [options] <commands.txt> [KEY=VALUE ...]\n"
		"   seqpipe log      [options]\n"
		"\n"
		"Try 'seqpipe help' to list all available subcommands.\n"
		<< std::endl;
}

int main(int argc, const char** argv)
{
	if (argc < 2) {
		PrintUsage();
		return 1;
	}
	const std::string name = argv[1];
	std::vector<std::string> args(argv + 2, argv + argc);

	if (name == "run") {
		CommandRun cmd;
		return cmd.Run(args);

	} else if (name == "parallel") {
		CommandParallel cmd;
		return cmd.Run(args);

	} else if (name == "log" || name == "history") {
		CommandLog cmd;
		return cmd.Run(args);

	} else if (name == "version") {
		std::cout << VERSION << std::endl;
		return 0;

	} else if (name == "help") {
		CommandHelp cmd;
		return cmd.Run(args);
	}

	CommandRun cmd; // Try as optional 'run' command
	args.insert(args.begin(), name);
	return cmd.Run(args);
}

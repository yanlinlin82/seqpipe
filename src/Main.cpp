#include <iostream>
#include <string>
#include <list>
#include "CommandRun.h"
#include "CommandRunParallel.h"
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
		"   seqpipe run   [options] <cmd> [args ...]\n"
		"   seqpipe [run] [options] <workflow.pipe> [procedure] [KEY=VALUE ...]\n"
		"   seqpipe log   [options]\n"
		"\n"
		"Try 'seqpipe help' to list all available subcommands.\n"
		<< std::endl;
}

int main(int argc, const char** argv)
{
	std::list<std::string> args(argv + 1, argv + argc);

	if (args.empty()) {
		PrintUsage();
		return 1;
	}

	const auto name = args.front();

	if (name == "run") {
		args.pop_front();
		CommandRun cmd;
		return cmd.Run(args);

	} else if (name == "prun" || name == "parallel") {
		args.pop_front();
		CommandRunParallel cmd;
		return cmd.Run(args);

	} else if (name == "log" || name == "history") {
		args.pop_front();
		CommandLog cmd;
		return cmd.Run(args);

	} else if (name == "version") {
		args.pop_front();
		std::cout << VERSION << std::endl;
		return 0;

	} else if (name == "help") {
		args.pop_front();
		CommandHelp cmd;
		return cmd.Run(args);
	}

	CommandRun cmd; // Try as optional 'run' command
	return cmd.Run(args);
}

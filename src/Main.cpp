#include <iostream>
#include <string>
#include <list>
#include "CommandRun.h"
#include "CommandLog.h"
#include "CommandRunDeprecated.h"
#include "SeqPipe.h"

static void PrintUsage()
{
	std::cout << "\n"
		"SeqPipe: a framework for SEQuencing data analysis PIPElines\n"
		"Version: " << VERSION << "\n"
		"Author : Linlin Yan (yanlinlin82<at>gmail.com)\n"
		"Website: https://github.com/yanlinlin82/seqpipe\n"
		"\n"
		"Usage:\n"
		"   seqpipe run [options] <workflow.pipe> [NAME=VALUE ...]\n"
		"   seqpipe run [options] <command> [arguments ...]\n"
		"   seqpipe log [options]\n"
		"   seqpipe version\n"
		"\n"
		"Deprecated Usage (Compatible to v0.4.x):\n"
		"   seqpipe [options] <procedure> [NAME=VALUE ...]\n"
		"   seqpipe [options] -e <cmd> [-e <cmd> ...] [NAME=VALUE ...]\n"
		"   seqpipe [options] -E <cmd> [-E <cmd> ...] [NAME=VALUE ...]\n"
		"\n"
		"Type 'seqpipe -h' to see help messages for deprecated usage.\n"
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

	} else if (name == "log") {
		args.pop_front();
		CommandLog cmd;
		return cmd.Run(args);

	} else if (name == "version") {
		args.pop_front();
		std::cout << VERSION << std::endl;
		return 0;
	}

	CommandRunDeprecated cmd;
	return cmd.Run(args);
}

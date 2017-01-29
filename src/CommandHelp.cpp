#include <iostream>
#include "CommandHelp.h"

void CommandHelp::PrintUsage()
{
	std::cout << "\n"
		"Usage: seqpipe [command] [options ...]\n"
		"\n"
		"Commands:\n"
		"   run               Run workflow\n"
		"   prun / parallel   Run workflow in parallel\n"
		"   log / history     Show history log\n"
		"   version           Show version\n"
		"   help              Show help messages\n"
		"\n"
		"Try 'seqpipe <command> -h' to see help messages for specific subcommand.\n"
		<< std::endl;
}

int CommandHelp::Run(const std::vector<std::string>& args)
{
	PrintUsage();
	return 0;
}

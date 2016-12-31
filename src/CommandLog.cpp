#include <iostream>
#include <cassert>
#include <unistd.h>
#include "CommandLog.h"
#include "System.h"
#include "Semaphore.h"

static const std::string LOG_ROOT = ".seqpipe";

void CommandLog::PrintUsage()
{
	std::cout << "\n"
		"Usage:\n"
		"   seqpipe log [list]\n"
		"   seqpipe log show [id_or_order]\n"
		"   seqpipe log remove [id_or_order]\n"
		"\n"
		"Options:\n"
		"   -h   Show this help messages.\n"
		"   -v   Show verbose messages.\n"
		<< std::endl;
}

bool CommandLog::ParseArgs(const std::list<std::string>& args)
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
				std::cerr << "Error: Unknown command '" << arg << "' for 'seqpipe log'!\n";
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

int CommandLog::Run(const std::list<std::string>& args)
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

int CommandLog::ListHistory()
{
	return System::Execute("cat " + LOG_ROOT + "/history.*.log | sort | less -X");
}

int CommandLog::ShowHistory()
{
	return System::Execute("cat " + LOG_ROOT + "/" + idOrOrder_ + "/log | less -X");
}

bool CommandLog::RelinkLastSymbolic(const std::string& uniqueId)
{
	const auto lastLink = LOG_ROOT + "/last";

	Semaphore sem("/seqpipe");
	std::lock_guard<Semaphore> lock(sem);

	if (System::CheckFileExists(lastLink)) {
		int retVal = unlink(lastLink.c_str());
		if (retVal != 0) {
			std::cerr << "Warning: Can not remove existed symbolic link '" + lastLink + "'! err: " << retVal << std::endl;
		}
	}

	if (!uniqueId.empty()) {
		int retVal = symlink(uniqueId.c_str(), lastLink.c_str());
		if (retVal != 0) {
			std::cerr << "Warning: Can not create symbolic link '" + lastLink + "' to '" << uniqueId << "'! err: " << retVal << std::endl;
		}
	}
	return true;
}

int CommandLog::RemoveHistory()
{
	const auto lastLink = LOG_ROOT + "/last";

	if (!System::CheckDirectoryExists(LOG_ROOT + "/" + idOrOrder_)) {
		std::cerr << "Error: Directory '" + LOG_ROOT + "/" + idOrOrder_ + "' does not exist!" << std::endl;
		return 1;
	}

	char path[1024] = "";
	readlink(lastLink.c_str(), path, sizeof(path));
	if (path == idOrOrder_) {
		unlink(lastLink.c_str());

	}
	return 1;
}

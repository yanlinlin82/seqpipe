#include <iostream>
#include <fstream>
#include <map>
#include <regex>
#include <cassert>
#include <unistd.h>
#include "CommandLog.h"
#include "System.h"
#include "Semaphore.h"
#include "SeqPipe.h"

void CommandLog::PrintUsage()
{
	std::cout << "\n"
		"Usage:\n"
		"   seqpipe log [list]\n"
		"   seqpipe log show [id_or_order]\n"
		"   seqpipe log remove [id_or_order]\n"
		"\n"
		"Options:\n"
		"   -h   Show help messages.\n"
		"   -v   Show verbose messages.\n"
		"\n"
		"Note: 'seqpipe history' is the synonym of 'seqpipe log'.\n"
		<< std::endl;
}

bool CommandLog::ParseArgs(const std::vector<std::string>& args)
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

int CommandLog::Run(const std::vector<std::string>& args)
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

bool CheckCmdResult(const std::string& id)
{
	auto filename = LOG_ROOT + "/" + id + "/log";
	std::ifstream file(filename);
	if (!file.is_open()) {
		return false;
	}
	std::string lastLine;
	std::string line;
	while (std::getline(file, line)) {
		lastLine = line;
	}
	return std::regex_search(lastLine, std::regex("Pipeline finished successfully"));
}

int CommandLog::ListHistory()
{
#if 0
	std::string filename = LOG_ROOT + "/history";
	return System::Execute("cat " + LOG_ROOT + "/history.*.log | sort | less -X");
#else
	std::vector<std::string> idList;
	std::map<std::string, std::pair<std::string, bool>> res; // id => { cmd, ok }

	const auto filenameList = System::ListFiles(LOG_ROOT, "history.*.log");
	for (const auto& filename : filenameList) {
		const auto fullname = LOG_ROOT + "/" + filename;

		std::ifstream file(fullname);
		if (!file.is_open()) {
			std::cerr << "Error: Can not open file '" << fullname << "' to read!" << std::endl;
			return 1;
		}
		std::string line;
		size_t lineNo = 0;
		while (std::getline(file, line)) {
			++lineNo;
			std::string::size_type pos = line.find_first_of('\t');
			if (pos == std::string::npos) {
				std::cerr << "Error: Invalid line in line " << lineNo << " of history file '" << fullname << "'!" << std::endl;
				return 1;
			}
			auto id = line.substr(0, pos);
			auto cmd = line.substr(pos + 1);
			bool ok = CheckCmdResult(id);
			idList.push_back(id);
			res[id] = std::make_pair(cmd, ok);
		}
		file.close();
	}

	std::sort(idList.begin(), idList.end());
	for (const auto& id : idList) {
		const auto& item = res[id];
		const auto& cmd = item.first;
		const auto ok = item.second;
		if (ok) {
			std::cout << "\033[00;32m";
		} else {
			std::cout << "\033[01;31m";
		}
		std::cout << id << "\t" << cmd;
		std::cout << "\033[00m";
		std::cout << "\n";
	}
	std::cout << std::flush;
	return 0;
#endif
}

int CommandLog::ShowHistory()
{
	return System::Execute("cat " + LOG_ROOT + "/" + idOrOrder_ + "/log | less -X");
}

bool CommandLog::RelinkLastSymbolic(const std::string& uniqueId)
{
	Semaphore sem("/seqpipe." + System::GetUserName());
	std::lock_guard<Semaphore> lock(sem);

	if (System::CheckFileExists(LOG_LAST)) {
		int retVal = unlink(LOG_LAST.c_str());
		if (retVal != 0) {
			std::cerr << "Warning: Can not remove existed symbolic link '" + LOG_LAST + "'! err: " << retVal << std::endl;
		}
	}

	if (!uniqueId.empty()) {
		int retVal = symlink(uniqueId.c_str(), LOG_LAST.c_str());
		if (retVal != 0) {
			std::cerr << "Warning: Can not create symbolic link '" + LOG_LAST + "' to '" << uniqueId << "'! err: " << retVal << std::endl;
		}
	}
	return true;
}

int CommandLog::RemoveHistory()
{
	if (!System::CheckDirectoryExists(LOG_ROOT + "/" + idOrOrder_)) {
		std::cerr << "Error: Directory '" + LOG_ROOT + "/" + idOrOrder_ + "' does not exist!" << std::endl;
		return 1;
	}

	char path[1024] = "";
	ssize_t n = readlink(LOG_LAST.c_str(), path, sizeof(path));
	if (n < 0) {
		std::cerr << "Error: Can not read link of '" << LOG_LAST << "'!" << std::endl;
		return 1;
	}

	if (path == idOrOrder_) {
		unlink(LOG_LAST.c_str());

	}
	return 1;
}

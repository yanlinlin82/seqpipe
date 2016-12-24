#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <sys/stat.h>
#include <unistd.h>

static int verbose = 0;
static int maxJobNumber = 0;
static bool forceRun = false;
static bool keepTemp = false;

static void PrintUsage()
{
	std::cout << "\n"
		"SeqPipe: A framework for establishing SEQuencing data analysis PIPElines\n"
		"Version: 0.5.0 (" GITVER ")\n"
		"Author : Linlin Yan (yanlinlin82<at>gmail.com)\n"
		"Website: https://github.com/yanlinlin82/seqpipe\n"
		"\n"
		"Usage:\n"
		"   seqpipe run [options] <workflow.pipe> [NAME=VALUE ...]\n"
		"   seqpipe run [options] <command> [arguments ...]\n"
		"   seqpipe history [options]\n"
		"\n"
		"Options:\n"
		"   -h / -H    Show help messages.\n"
		"   -v         Show verbose messages.\n"
		"   -t <int>   Max job number in parallel. default as current processor number.\n"
		"   -f         Force to re-run when output files are already latest.\n"
		"   -k         Keep temporary files.\n"
		<< std::endl;
}

static bool CheckFileExists(const std::string& path)
{
	struct stat buffer;   
	return (stat(path.c_str(), &buffer) == 0); 
}

static bool CheckDirectoryExists(const std::string& path)
{
	struct stat buffer;   
	return (stat(path.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode)); 
}

static std::string GetHostname()
{
	char buffer[256] = "";
	gethostname(buffer, sizeof(buffer));
	return buffer;
}

static std::string GetUniqueId(const std::string& hostname)
{
	char buffer[64] = "";
	time_t now = time(NULL);
	struct tm tmBuf;
	localtime_r(&now, &tmBuf);
	snprintf(buffer, sizeof(buffer), "%02d%02d%02d.%02d%02d.%d.",
			tmBuf.tm_year % 100, tmBuf.tm_mon + 1, tmBuf.tm_mday,
			tmBuf.tm_hour, tmBuf.tm_min, getpid());
	return (buffer + hostname);
}

static std::string CommandEncode(const std::string& s)
{
	if (s.find_first_of(" \t") != std::string::npos) {
		return "'" + s + "'";
	} else {
		return s;
	}
}

static int ProcessRun(const std::list<std::string>& args, const std::string& cmdLine)
{
	std::string command;
	bool commandIsPipeFile = false;
	std::vector<std::string> arguments;

	for (auto it = args.begin(); it != args.end(); ++it) {
		const auto& arg = *it;
		if ((command.empty() || commandIsPipeFile) && arg[0] == '-') {
			if (arg == "-h" || arg == "-H") {
				PrintUsage();
				return 1;
			} else if (arg == "-v") {
				++verbose;
			} else if (arg == "-t") {
				const auto& parameter = *(++it);
				maxJobNumber = std::stoi(parameter);
				if (maxJobNumber < 0) {
					std::cerr << "Error: Invalid number '" << parameter << "' for option '-t'!\n";
					return 1;
				}
			} else if (arg == "-f") {
				forceRun = true;
			} else if (arg == "-k") {
				keepTemp = true;
			} else {
				std::cerr << "Error: Unknown option '" << arg << "'!\n";
				return 1;
			}
		} else if (command.empty()) {
			command = arg;
			commandIsPipeFile = CheckFileExists(command);
		} else {
			arguments.push_back(arg);
		}
	}

	if (verbose > 0) {
		if (commandIsPipeFile) {
			std::cerr << "Run pipe file: '" << command << "' with:\n";
		} else {
			std::cerr << "Run command: '" << command << "' with:\n";
		}
		for (const auto arg : arguments) {
			std::cerr << "  [" << arg << "]" << std::endl;
		}
	}

	std::string hostname = GetHostname();
	std::string uniqueId = GetUniqueId(hostname);
	std::string logRoot = ".seqpipe";
	std::string logDir = logRoot + "/" + uniqueId;

	if (!CheckDirectoryExists(logRoot)) {
		if (verbose > 0) {
			std::cerr << "Create directory: '" << logRoot << "'" << std::endl;
		}
		mkdir(logRoot.c_str(), 0755);
	}
	if (!CheckDirectoryExists(logDir)) {
		if (verbose > 0) {
			std::cerr << "Create directory: '" << logDir << "'" << std::endl;
		}
		mkdir(logDir.c_str(), 0755);
	}

	// TODO: lock for 'history.log'
	std::string historyLog = logRoot + "/history." + hostname + ".log";
	std::ofstream file(historyLog, std::ios::app);
	if (!file.is_open()) {
		std::cerr << "Error: Can not write to history file '" << historyLog << "'!" << std::endl;
		return 1;
	}
	file << uniqueId << '\t' << cmdLine << std::endl;
	file.close();

	// TODO: lock for 'last'
	std::string lastLink = logRoot + "/last";
	if (CheckFileExists(lastLink)) {
		unlink(lastLink.c_str());
	}
	int retVal = symlink(uniqueId.c_str(), lastLink.c_str());
	if (retVal != 0) {
		std::cerr << "Warning: Can not create symbolic link '.seqpipe/last' to '" << uniqueId << "'! err: " << retVal << std::endl;
	}

	if (commandIsPipeFile) {
		std::cerr << "Error: Unimplemented!\n";
		return 1;
	} else {
		std::string cmdLine = command;
		for (const auto arg : arguments) {
			cmdLine += ' ' + CommandEncode(arg);
		}
		int retVal = system(cmdLine.c_str());
		if (retVal != 0) {
			return retVal;
		}
	}
	return 0;
}

static int ProcessHistory(const std::list<std::string>& args)
{
	// TODO: implementation
	return 0;
}

std::string JointCommandLine(int argc, const char** argv)
{
	// TODO: process quotes, multilines, special characters
	std::string cmdLine = CommandEncode(argv[0]);
	for (int i = 1; i < argc; ++i) {
		cmdLine += ' ';
		cmdLine += CommandEncode(argv[i]);
	}
	return cmdLine;
}

int main(int argc, const char** argv)
{
	std::list<std::string> args(argv + 1, argv + argc);

	if (args.empty()) {
		PrintUsage();
		return 1;
	}

	if (args.front() == "run") {
		args.pop_front();
		return ProcessRun(args, JointCommandLine(argc, argv));
	} else if (args.front() == "history") {
		args.pop_front();
		return ProcessHistory(args);
	}

	std::cerr << "Error: Unknown command '" << args.front() << "'" << std::endl;
	return 1;
}

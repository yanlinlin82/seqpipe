#include <fstream>
#include "Launcher.h"
#include "System.h"
#include "StringUtils.h"

bool Launcher::CheckIfPipeFile(const std::string& command)
{
	if (!System::CheckFileExists(command)) {
		return false;
	}
	if (!System::IsTextFile(command)) {
		return false;
	}
	return true;
}

static std::string GetFirstWord(const std::string& s)
{
	std::string::size_type pos = s.find_first_of(" \t\n\r");
	if (pos != std::string::npos) {
		return s.substr(0, pos);
	}
	return s;
}

bool Launcher::LoadPipeFile(const std::string& filename)
{
	std::ifstream file(filename);
	if (!file) {
		return false;
	}

	std::string line;
	while (std::getline(file, line)) {
		CommandItem item;
		item.name_ = GetFirstWord(line);
		item.cmdLine_ = line;
		commandLines_.push_back(item);
	}

	file.close();
	return true;
}

std::string Launcher::JoinCommandLine(const std::string& cmd, const std::vector<std::string>& arguments)
{
	std::string cmdLine = cmd;
	for (const auto arg : arguments) {
		cmdLine += ' ' + System::EncodeShell(arg);
	}
	return cmdLine;
}

bool Launcher::AppendCommand(const std::string& cmd, const std::vector<std::string>& arguments)
{
	CommandItem item;
	item.name_ = cmd;
	item.cmdLine_ = JoinCommandLine(cmd, arguments);
	commandLines_.push_back(item);
	return true;
}

static void WriteFile(const std::string& filename, const std::string& s)
{
	std::ofstream file(filename);
	file << s << std::endl;
	file.close();
}

int Launcher::Run(LogFile& logFile, const std::string& logDir, int verbose)
{
	for (size_t i = 0; i < commandLines_.size(); ++i) {
		const std::string name = std::to_string(i + 1) + "." + StringUtils::RemoveSpecialCharacters(commandLines_[i].name_);
		const auto& cmdLine = commandLines_[i].cmdLine_;

		logFile.WriteLine(Msg() << "(" << i + 1 << ") [shell] " << cmdLine);
		time_t t0 = time(NULL);
		logFile.WriteLine(Msg() << "(" << i + 1 << ") starts at " << StringUtils::TimeString(t0));

		WriteFile(logDir + "/" + name + ".cmd", cmdLine);

		std::string fullCmdLine = "( " + cmdLine + " )";
		if (verbose > 0) {
			fullCmdLine += " 2> >(tee -a " + logDir + "/" + name + ".err >&2)";
			fullCmdLine += " > >(tee -a " + logDir + "/" + name + ".log)";
		} else {
			fullCmdLine += " 2>>" + logDir + "/" + name + ".err";
			fullCmdLine += " >>" + logDir + "/" + name + ".log";
		}
		int retVal = System::Execute(fullCmdLine.c_str());

		time_t t = time(NULL);
		logFile.WriteLine(Msg() << "(" << i + 1 << ") ends at " << StringUtils::TimeString(t) << " (elapsed: " << StringUtils::DiffTimeString(t - t0) << ")");
	}
	return 0;
}

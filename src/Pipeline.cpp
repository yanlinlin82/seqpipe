#include <fstream>
#include <regex>
#include <cassert>
#include "Pipeline.h"
#include "StringUtils.h"
#include "System.h"

bool Pipeline::CheckIfPipeFile(const std::string& command)
{
	if (!System::CheckFileExists(command)) {
		return false;
	}
	if (System::HasExecutiveAttribute(command)) {
		return false;
	}
	if (!System::IsTextFile(command)) {
		return false;
	}
	return true;
}

bool Pipeline::Load(const std::string& filename)
{
	assert(originPipeline_.empty());

	std::ifstream file(filename);
	if (!file) {
		return false;
	}

	std::string line;
	while (std::getline(file, line)) {
		originPipeline_.push_back(line);
		if (std::regex_match(line, std::regex("^\\s*#"))) {
			continue;
		}
		CommandItem item;
		item.name_ = StringUtils::GetFirstWord(line);
		item.cmdLine_ = line;
		commandLines_.push_back(item);
	}

	file.close();
	return true;
}

bool Pipeline::Save(const std::string& filename) const
{
	std::ofstream file(filename);
	if (!file) {
		return false;
	}

	for (const auto& s : originPipeline_) {
		file << s << std::endl;
	}

	file.close();
	return true;
}

std::string Pipeline::JoinCommandLine(const std::string& cmd, const std::vector<std::string>& arguments)
{
	std::string cmdLine = cmd;
	for (const auto arg : arguments) {
		cmdLine += ' ' + System::EncodeShell(arg);
	}
	return cmdLine;
}

bool Pipeline::AppendCommand(const std::string& cmd, const std::vector<std::string>& arguments)
{
	auto cmdLine = JoinCommandLine(cmd, arguments);
	originPipeline_.push_back(cmdLine);

	CommandItem item;
	item.name_ = cmd;
	item.cmdLine_ = cmdLine;
	commandLines_.push_back(item);
	return true;
}

#include <iostream>
#include <fstream>
#include <map>
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

static bool IsEmptyLine(const std::string& s)
{
	return std::regex_match(s, std::regex("^\\s*$"));
}

static bool IsCommentLine(const std::string& s)
{
	return std::regex_match(s, std::regex("^\\s*#$"));
}

static bool IsDescLine(const std::string& s)
{
	return std::regex_match(s, std::regex("^\\s*#\\["));
}

static bool ParseAttrLine(const std::string& s)
{
	return std::regex_match(s, std::regex("^\\s*#\\[(\\w+\\s+|)(\\s*([\\w\\.]+)=\"[^\"]*\"(\\s+([\\w\\.]+)=\"[^\"]*\")*|)\\s*\\]\\s*$"));
}

static bool IsIncLine(const std::string& s, std::string& filename)
{
	std::smatch sm;
	if (!std::regex_match(s, sm, std::regex("^\\s*(SP_include|source|\\.)\\s+(.*)\\s*$"))) {
		return false;
	}
	filename = sm[2];
	return true;
}

static bool IsVarLine(const std::string& s, std::string& name, std::string& value)
{
	std::smatch sm;
	if (!std::regex_match(s, sm, std::regex("^\\s*([\\w\\.]+)\\s*=(.*)$"))) {
		return false;
	}
	name = sm[1];
	value = sm[2];
	return true;
}

static bool IsFuncLine(const std::string& s, std::string& name, std::string& leftBracket)
{
	std::smatch sm;
	if (std::regex_match(s, sm, std::regex("^\\s*function\\s+([\\w\\.]+)\\s*(\\{|\\{\\{|)\\s*$"))) {
		name = sm[1];
		leftBracket = sm[2];
		return true;
	}
	if (std::regex_match(s, sm, std::regex("^\\s*([\\w\\.]+)\\s*\\(\\s*\\)\\s*(\\{|\\{\\{|)\\s*$"))) {
		name = sm[1];
		leftBracket = sm[2];
		return true;
	}
	return false;
}

std::vector<std::string> Pipeline::GetProcNameList() const
{
	std::vector<std::string> nameList;
	for (auto it = procList_.begin(); it != procList_.end(); ++it) {
		nameList.push_back(it->first);
	}
	return nameList;
}

static bool IsLeftBracket(const std::string& s, std::string& leftBracket)
{
	std::smatch sm;
	if (!std::regex_match(s, sm, std::regex("^\\s*({|{{)\\s*$"))) {
		return false;
	}
	leftBracket = sm[1];
	return true;
}

static bool IsRightBracket(const std::string& s, std::string& rightBracket)
{
	std::smatch sm;
	if (!std::regex_match(s, sm, std::regex("^\\s*(}|}})\\s*$"))) {
		return false;
	}
	rightBracket = sm[1];
	return true;
}

bool Pipeline::LoadProc(PipeFile& file, const std::string& name, std::string leftBracket, Procedure& proc)
{
	auto it = procList_.find(name);
	if (it != procList_.end()) {
		std::cerr << "ERROR: Duplicated procedure '" << name << "' in " << file.Pos() << "\n"
			"   Previous definition of '" << name << "' was in " << it->second.Pos() << std::endl;
		return false;
	}

	if (leftBracket.empty()) {
		while (file.ReadLine()) {
			if (IsEmptyLine(file.CurrentLine())) {
				continue;
			} else if (IsCommentLine(file.CurrentLine())) {
				if (IsDescLine(file.CurrentLine())) {
					std::cerr << "ERROR: Unexpected attribute line in " << file.Pos() << std::endl;
					return false;
				}
				continue;
			} else if (!IsLeftBracket(file.CurrentLine(), leftBracket)) {
				std::cerr << "ERROR: Unexpected line in " << file.Pos() << "\n"
					"   Only '{' or '{{' was expected here." << std::endl;
				return false;
			}
			break;
		}
	}

	procList_[name].SetName(name);
	while (file.ReadLine()) {
		std::string rightBracket;
		if (IsRightBracket(file.CurrentLine(), rightBracket)) {
			if (leftBracket == "{" && rightBracket == "}}") {
				std::cerr << "ERROR: Unexpected right bracket in " << file.Pos() << "\n"
					"   Right bracket '}' was expected here." << std::endl;
				return false;
			} else if (leftBracket == "{{" && rightBracket == "}") {
				std::cerr << "ERROR: Unexpected right bracket in " << file.Pos() << "\n"
					"   Right bracket '}}' was expected here." << std::endl;
				return false;
			}
			break;
		} else {
			procList_[name].AppendCommand(file.CurrentLine());
		}
	}

	return true;
}

bool Pipeline::LoadConf(const std::string& filename, std::map<std::string, std::string>& confMap)
{
	std::ifstream file(filename);
	if (!file.is_open()) {
		return false;
	}

	std::string line;
	size_t lineNo = 0;
	while (std::getline(file, line)) {
		++lineNo;
		std::string name;
		std::string value;
		if (IsVarLine(line, name, value)) {
			confMap[name] = value;
		} else {
			if (!IsEmptyLine(line) && !IsCommentLine(line)) {
				std::cerr << "ERROR: Invalid syntax of configure file in " << filename << "(" << lineNo << ")\n"
					"  Only global variable definition could be included in configure file!" << std::endl;
				return false;
			}
		}
	}
	file.close();
	return true;
}

bool Procedure::AppendCommand(const std::string& line)
{
	std::string cmd;
	std::vector<std::string> arguments;
	if (!StringUtils::ParseCommandLine(line, cmd, arguments)) {
		return false;
	}
	return AppendCommand(cmd, arguments);
}

static std::string JoinCommandLine(const std::string& cmd, const std::vector<std::string>& arguments)
{
	std::string cmdLine = cmd;
	for (const auto arg : arguments) {
		cmdLine += ' ' + System::EncodeShell(arg);
	}
	return cmdLine;
}

bool Procedure::AppendCommand(const std::string& cmd, const std::vector<std::string>& arguments)
{
	CommandItem item;
	item.name_ = cmd;
	item.cmdLine_ = JoinCommandLine(cmd, arguments);
	commandLines_.push_back(item);
	return true;
}

bool Pipeline::Load(const std::string& filename)
{
	std::map<std::string, std::string> confMap;

	PipeFile file;
	if (!file.Open(filename)) {
		return false;
	}
	while (file.ReadLine()) {

		if (IsEmptyLine(file.CurrentLine())) {
			continue;
		}
		if (IsCommentLine(file.CurrentLine())) {
			if (IsDescLine(file.CurrentLine())) {
				if (!ParseAttrLine(file.CurrentLine())) {
					std::cerr << "WARNING: Invalid format of attribute in " << file.Pos() << "!" << std::endl;
				}
			}
			continue;
		}

		std::string includeFilename;
		if (IsIncLine(file.CurrentLine(), includeFilename)) {
			std::cerr << "Loading module '" << includeFilename << "'" << std::endl;
			if (!LoadConf(System::DirName(file.Filename()) + "/" + includeFilename, confMap)) {
				return false;
			}
			continue;
		}

		std::string name;
		std::string value;
		if (IsVarLine(file.CurrentLine(), name, value)) {
			confMap[name] = value;
		}

		std::string leftBracket;
		if (IsFuncLine(file.CurrentLine(), name, leftBracket)) {
			Procedure proc;
			if (!LoadProc(file, name, leftBracket, proc)) {
				return false;
			}
			continue;
		}

		if (!defaultProc_.AppendCommand(file.CurrentLine())) {
			return false;
		}
	}

	auto confFilename = filename + ".conf";
	if (System::CheckFileExists(confFilename)) {
		if (!LoadConf(confFilename, confMap)) {
			return false;
		}
	}
	return true;
}

bool Pipeline::Save(const std::string& filename) const
{
	std::ofstream file(filename);
	if (!file) {
		return false;
	}

	bool first = true;
	for (auto it = procList_.begin(); it != procList_.end(); ++it) {
		if (first) {
			first = false;
		} else {
			file << "\n";
		}

		file << it->first << "() {\n";
		for (const auto& cmd : it->second.GetCommandLines()) {
			file << "\t" << cmd.cmdLine_ << "\n";
		}
		file << "}\n";
	}

	if (!defaultProc_.GetCommandLines().empty()) {
		file << "\n";
		for (const auto& cmd : defaultProc_.GetCommandLines()) {
			file << cmd.cmdLine_ << "\n";
		}
	}

	file.close();
	return true;
}

bool Pipeline::AppendCommand(const std::string& cmd, const std::vector<std::string>& arguments)
{
	return defaultProc_.AppendCommand(cmd, arguments);
}

const Procedure* Pipeline::GetProc(const std::string& name) const
{
	if (name.empty()) {
		if (defaultProc_.GetCommandLines().empty()) {
			std::cerr << "ERROR: No procedure name provided!" << std::endl;
			return NULL;
		}
		return &defaultProc_;
	} else {
		auto it = procList_.find(name);
		if (it == procList_.end()) {
			std::cerr << "ERROR: Can not find procedure '" << name << "'!" << std::endl;
			return NULL;
		}
		return &it->second;
	}
}

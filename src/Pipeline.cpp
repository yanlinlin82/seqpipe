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

std::vector<std::string> Pipeline::GetProcNameList() const
{
	std::vector<std::string> nameList;
	for (auto it = procList_.begin(); it != procList_.end(); ++it) {
		nameList.push_back(it->first);
	}
	return nameList;
}

bool Pipeline::LoadProc(PipeFile& file, const std::string& name, std::string leftBracket, Procedure& proc)
{
	if (leftBracket.empty()) {
		while (file.ReadLine()) {
			if (PipeFile::IsEmptyLine(file.CurrentLine())) {
				continue;
			} else if (PipeFile::IsCommentLine(file.CurrentLine())) {
				if (PipeFile::IsDescLine(file.CurrentLine())) {
					std::cerr << "Error: Unexpected attribute line at " << file.Pos() << std::endl;
					return false;
				}
				continue;
			} else if (!PipeFile::IsLeftBracket(file.CurrentLine(), leftBracket)) {
				std::cerr << "Error: Unexpected line at " << file.Pos() << "\n"
					"   Only '{' or '{{' was expected here." << std::endl;
				return false;
			}
			break;
		}
	}

	procList_[name].SetName(name);
	while (file.ReadLine()) {
		std::string rightBracket;
		if (PipeFile::IsRightBracket(file.CurrentLine(), rightBracket)) {
			if (leftBracket == "{" && rightBracket == "}}") {
				std::cerr << "Error: Unexpected right bracket at " << file.Pos() << "\n"
					"   Right bracket '}' was expected here." << std::endl;
				return false;
			} else if (leftBracket == "{{" && rightBracket == "}") {
				std::cerr << "Error: Unexpected right bracket at " << file.Pos() << "\n"
					"   Right bracket '}}' was expected here." << std::endl;
				return false;
			}
			break;
		} else {
			procList_[name].AppendCommand(file.CurrentLine());
		}
	}
	bool parallel = (leftBracket == "{{");
	procList_[name].SetParallel(parallel);

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
		if (PipeFile::IsVarLine(line, name, value)) {
			confMap[name] = value;
		} else {
			if (!PipeFile::IsEmptyLine(line) && !PipeFile::IsCommentLine(line)) {
				std::cerr << "Error: Invalid syntax of configure file in " << filename << "(" << lineNo << ")\n"
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
	block_.items_.push_back(item);
	return true;
}

bool Pipeline::Load(const std::string& filename)
{
	std::map<std::string, std::string> confMap;
	std::map<std::string, std::string> procAtLineNo;

	PipeFile file;
	if (!file.Open(filename)) {
		return false;
	}
	while (file.ReadLine()) {

		if (PipeFile::IsEmptyLine(file.CurrentLine())) {
			continue;
		}
		if (PipeFile::IsCommentLine(file.CurrentLine())) {
			if (PipeFile::IsDescLine(file.CurrentLine())) {
				if (!PipeFile::ParseAttrLine(file.CurrentLine())) {
					std::cerr << "Warning: Invalid format of attribute at " << file.Pos() << "!" << std::endl;
				}
			}
			continue;
		}

		std::string includeFilename;
		if (PipeFile::IsIncLine(file.CurrentLine(), includeFilename)) {
			std::cerr << "Loading module '" << includeFilename << "'" << std::endl;
			if (!LoadConf(System::DirName(file.Filename()) + "/" + includeFilename, confMap)) {
				return false;
			}
			continue;
		}

		std::string name;
		std::string value;
		if (PipeFile::IsVarLine(file.CurrentLine(), name, value)) {
			confMap[name] = value;
		}

		std::string leftBracket;
		if (PipeFile::IsFuncLine(file.CurrentLine(), name, leftBracket)) {
			if (procAtLineNo.find(name) != procAtLineNo.end()) {
				std::cerr << "Error: Duplicated procedure '" << name << "' at " << file.Pos() << "\n"
					"   Previous definition of '" << name << "' was in " << procAtLineNo[name] << std::endl;
				return false;
			}
			procAtLineNo[name] = file.Pos();

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
		for (const auto& cmd : it->second.GetBlock().items_) {
			file << "\t" << cmd.cmdLine_ << "\n";
		}
		file << "}\n";
	}

	if (!defaultProc_.GetBlock().items_.empty()) {
		if (!procList_.empty()) {
			file << "\n";
		}
		for (const auto& cmd : defaultProc_.GetBlock().items_) {
			file << cmd.cmdLine_ << "\n";
		}
	}

	file.close();
	return true;
}

bool Pipeline::SetDefaultProc(const std::vector<std::string>& cmdList, bool parallel)
{
	assert(defaultProc_.GetBlock().items_.empty());

	for (const auto& cmd : cmdList) {
		defaultProc_.AppendCommand(cmd, {});
	}
	defaultProc_.SetParallel(parallel);
}

bool Pipeline::AppendCommand(const std::string& cmd, const std::vector<std::string>& arguments)
{
	return defaultProc_.AppendCommand(cmd, arguments);
}

bool Pipeline::HasProcedure(const std::string& name) const
{
	return procList_.find(name) != procList_.end();
}

Block Pipeline::GetBlock(const std::string& procName) const
{
	if (procName.empty()) {
		return defaultProc_.GetBlock();
	} else {
		auto it = procList_.find(procName);
		if (it == procList_.end()) {
			throw std::runtime_error("Invalid procName");
		}
		return it->second.GetBlock();
	}
}

bool Pipeline::HasAnyDefaultCommand() const
{
	return !defaultProc_.GetBlock().items_.empty();
}

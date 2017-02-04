#include <iostream>
#include <fstream>
#include <map>
#include <regex>
#include <cassert>
#include "Pipeline.h"
#include "StringUtils.h"
#include "System.h"
#include "CommandLineParser.h"

std::string ProcArgs::Get(const std::string& key) const
{
	auto it = args_.find(key);
	return (it == args_.end() ? "" : it->second);
}

void ProcArgs::Add(const std::string& key, const std::string& value)
{
	assert(!Has(key));
	args_[key] = value;
	order_.push_back(key);
}

std::string ProcArgs::ToString() const
{
	std::string s;
	assert(args_.size() == order_.size());
	for (auto name : order_) {
		auto it = args_.find(name);
		assert(it != args_.end());
		s += " " + name + "=" + System::EncodeShell(it->second, false);
	}
	return s;
}

void ProcArgs::Clear()
{
	args_.clear();
	order_.clear();
}

CommandItem::CommandItem(const std::string& cmd, const std::vector<std::string>& arguments):
	type_(CommandType::TYPE_SHELL), shellCmd_(cmd), shellArgs_(arguments)
{
	name_ = StringUtils::RemoveSpecialCharacters(cmd);
	fullCmdLine_ = cmd;
	for (const auto arg : arguments) {
		fullCmdLine_ += ' ' + System::EncodeShell(arg, false);
	}
}

CommandItem::CommandItem(const std::string& procName, const ProcArgs& procArgs):
	type_(CommandType::TYPE_PROC), procArgs_(procArgs), procName_(procName)
{
	name_ = procName;
}

CommandItem::CommandItem(size_t blockIndex):
	type_(CommandType::TYPE_BLOCK), blockIndex_(blockIndex)
{
}

CommandItem::CommandItem(const std::string& cmdLine):
	type_(CommandType::TYPE_SHELL), fullCmdLine_(cmdLine)
{
}

const std::string& CommandItem::CmdLine() const
{
	assert(type_ == CommandType::TYPE_SHELL);
	return fullCmdLine_;
}

const std::string& CommandItem::ShellCmd() const
{
	assert(type_ == CommandType::TYPE_SHELL);
	return shellCmd_;
}

const std::string& CommandItem::ProcName() const
{
	assert(type_ == CommandType::TYPE_PROC);
	return procName_;
}

const ProcArgs& CommandItem::GetProcArgs() const
{
	assert(type_ == CommandType::TYPE_PROC);
	return procArgs_;
}

size_t CommandItem::GetBlockIndex() const
{
	assert(type_ == CommandType::TYPE_BLOCK);
	return blockIndex_;
}

std::string CommandItem::ToString() const
{
	if (type_ == CommandType::TYPE_SHELL) {
		return fullCmdLine_;
	} else {
		return procName_ + procArgs_.ToString();
	}
}

std::string CommandItem::ToString(const std::string& indent, const Pipeline& pipeline) const
{
	if (Type() == CommandType::TYPE_BLOCK) {
		return pipeline.GetBlock(blockIndex_).ToString(indent, pipeline);
	} else {
		return indent + ToString() + "\n";
	}
}

void CommandItem::Dump(const std::string& indent, const Pipeline& pipeline) const
{
	std::cout << ToString(indent, pipeline);
}

std::ostream& operator << (std::ostream& os, CommandType type)
{
	if (type == CommandType::TYPE_SHELL) {
		os << "shell";
	} else if (type == CommandType::TYPE_PROC) {
		os << "proc";
	} else if (type == CommandType::TYPE_BLOCK) {
		os << "block";
	}
	return os;
}

std::string CommandItem::DetailToString() const
{
	std::ostringstream ss;

	ss << "type='" << type_ << "'";
	ss << ", name='" << name_ << "'";
	ss << ", procArgs={" << procArgs_.ToString() << "}";

	ss << ", fullCmdLine='" << fullCmdLine_ << "'";
	ss << ", shellCmd='" << shellCmd_ << "'";
	ss << ", shellArgs={";
	for (size_t i = 0; i < shellArgs_.size(); ++i) {
		ss << (i == 0 ? "" : " ") << "'" << shellArgs_[i] << "'";
	}
	ss << "}";

	ss << ", procName='" << procName_ << "'";

	ss << ", blockIndex=" << blockIndex_;

	return ss.str();
}

void Block::Clear()
{
	items_.clear();
	parallel_ = false;
}

void Block::AppendCommand(const std::string& cmd, const std::vector<std::string>& arguments)
{
	items_.push_back(CommandItem(cmd, arguments));
}

void Block::AppendCommand(const std::vector<std::vector<std::string>>& argLists, Pipeline& pipeline)
{
	if (argLists.size() == 1 || !parallel_) {
		for (const auto& argList : argLists) {
			assert(!argList.empty());
			const auto& cmd = argList[0];
			const auto args = std::vector<std::string>(argList.begin() + 1, argList.end());
			AppendCommand(cmd, args);
		}
	} else if (!argLists.empty()) {
		Block block;
		for (const auto& argList : argLists) {
			assert(!argList.empty());
			const auto& cmd = argList[0];
			const auto args = std::vector<std::string>(argList.begin() + 1, argList.end());
			block.AppendCommand(cmd, args);
		}
		size_t blockIndex = pipeline.AppendBlock(block);
		AppendBlock(blockIndex);
	}
}

void Block::AppendCommand(const std::string& procName, const ProcArgs& procArgs)
{
	items_.push_back(CommandItem(procName, procArgs));
}

bool Block::AppendBlock(size_t blockIndex)
{
	items_.push_back(CommandItem(blockIndex));
	return true;
}

bool Block::UpdateCommandToProcCalling(const std::set<std::string>& procNameSet)
{
	for (auto& item : items_) {
		if (item.Type() == CommandType::TYPE_SHELL
				&& procNameSet.find(item.ShellCmd()) != procNameSet.end()) {
			if (!item.ConvertShellToProc()) {
				return false;
			}
		}
	}
	return true;
}

std::string Block::ToString(const std::string& indent, const Pipeline& pipeline) const
{
	std::string s;
	s += indent + (parallel_ ? "{{" : "{") + "\n";
	for (const auto& item : items_) {
		s += item.ToString(indent + "\t", pipeline);
	}
	s += indent + (parallel_ ? "}}" : "}") + "\n";
	return s;
}

std::string Block::DetailToString() const
{
	if (items_.empty()) {
		return "<empty>";
	} else if (items_.size() == 1) {
		return items_[0].DetailToString();
	} else {
		std::ostringstream ss;
		ss << " (parallel = " << parallel_ << ") " << items_.size() << " items:\n";
		for (size_t i = 0; i < items_.size(); ++i) {
			if (i > 0) {
				ss << "\n";
			}
			ss << "  [" << i << "]" << items_[i].DetailToString();
		}
		return ss.str();
	}
}

void Block::Dump(const std::string& indent, const Pipeline& pipeline) const
{
	std::cout << ToString(indent, pipeline);
}

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

std::vector<std::string> Pipeline::GetProcNameList(const std::string& pattern) const
{
	std::vector<std::string> nameList;
	for (auto it = procList_.begin(); it != procList_.end(); ++it) {
		const auto& name = it->first;
		if (std::regex_search(name, std::regex(pattern))) {
			nameList.push_back(it->first);
		}
	}
	return nameList;
}

bool Pipeline::ReadLeftBracket(PipeFile& file, std::string& leftBracket)
{
	for (;;) {
		if (PipeFile::IsEmptyLine(file.CurrentLine())) {
			; // to read next line
		} else if (PipeFile::IsCommentLine(file.CurrentLine())) {
			if (PipeFile::IsDescLine(file.CurrentLine())) {
				std::cerr << "Error: Unexpected attribute line at " << file.Pos() << std::endl;
				return false;
			}
			; // to read next line
		} else if (!PipeFile::IsLeftBracket(file.CurrentLine(), leftBracket)) {
			std::cerr << "Error: Unexpected line at " << file.Pos() << "\n"
				"   Only '{' or '{{' was expected here." << std::endl;
			return false;
		} else {
			return true;
		}

		if (!file.ReadLine()) {
			std::cerr << "Error: Missing left bracket for procedure declare at" << file.Pos() << std::endl;
			return false;
		}
	}
}

bool Pipeline::LoadBlock(PipeFile& file, Block& block, bool parallel)
{
	block.SetParallel(parallel);
	for (;;) {
		std::string rightBracket;
		if (PipeFile::IsRightBracket(file.CurrentLine(), rightBracket)) {
			if (!parallel && rightBracket == "}}") {
				std::cerr << "Error: Unexpected right bracket at " << file.Pos() << "\n"
					"   Right bracket '}' was expected here." << std::endl;
				return false;
			} else if (parallel && rightBracket == "}") {
				std::cerr << "Error: Unexpected right bracket at " << file.Pos() << "\n"
					"   Right bracket '}}' was expected here." << std::endl;
				return false;
			}
			break;
		}

		std::string leftBracket;
		if (PipeFile::IsLeftBracket(file.CurrentLine(), leftBracket)) {
			if (!file.ReadLine()) {
				return false;
			}
			Block subBlock;
			if (!LoadBlock(file, subBlock, (leftBracket == "{{"))) {
				return false;
			}
			size_t blockIndex = AppendBlock(subBlock);
			if (!block.AppendBlock(blockIndex)) {
				return false;
			}
			if (!file.ReadLine()) {
				std::cerr << "Missing right bracket '" << (parallel ? "}}" : "}") << "' at " << file.Pos() << std::endl;
				return false;
			}
			continue;
		}

		if (!AppendCommandLineFromFile(file, block)) {
			return false;
		}
		if (!file.ReadLine()) {
			std::cerr << "Missing right bracket '" << (parallel ? "}}" : "}") << "' at " << file.Pos() << std::endl;
			return false;
		}
	}
	return true;
}

bool Pipeline::AppendCommandLineFromFile(PipeFile& file, Block& block)
{
	std::string lines = file.CurrentLine();
	CommandLineParser parser;
	for (;;) {
		if (!parser.Parse(lines)) {
			if (parser.IsUnfinished()) {
				if (!file.ReadLine()) {
					std::cerr << "Unexpected EOF at " << file.Pos() << std::endl;
					return false;
				}
				lines += "\n" + file.CurrentLine();
				continue;
			}
			std::cerr << "Error when parsing shell command at " << file.Pos() << std::endl;
			return false;
		}
		block.AppendCommand(parser.GetArgLists(), *this);
		break;
	}
	return true;
}

bool Pipeline::LoadProc(PipeFile& file, const std::string& name, std::string leftBracket, Procedure& proc)
{
	if (leftBracket.empty()) {
		if (!file.ReadLine()) {
			return false;
		}
		if (!ReadLeftBracket(file, leftBracket)) {
			return false;
		}
	}

	Block block;
	if (!LoadBlock(file, block, (leftBracket == "{{"))) {
		return false;
	}
	size_t blockIndex = blockList_.size();
	blockList_.push_back(block);

	procList_[name].Initialize(name, blockIndex);
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

bool Pipeline::Load(const std::string& filename)
{
	std::map<std::string, std::string> confMap;

	PipeFile file;
	if (!file.Open(filename)) {
		return false;
	}
	if (file.ReadLine()) {
		for (;;) {
			{ // try include line
				std::string includeFilename;
				if (PipeFile::IsIncLine(file.CurrentLine(), includeFilename)) {
					std::cerr << "Loading module '" << includeFilename << "'" << std::endl;
					if (!LoadConf(System::DirName(file.Filename()) + "/" + includeFilename, confMap)) {
						return false;
					}
					if (!file.ReadLine()) {
						break;
					}
					continue;
				}
			}

			{ // try function line
				std::string procName;
				std::string leftBracket;
				if (PipeFile::IsFuncLine(file.CurrentLine(), procName, leftBracket)) {
					if (procAtLineNo_.find(procName) != procAtLineNo_.end()) {
						std::cerr << "Error: Duplicated procedure '" << procName << "' at " << file.Pos() << "\n"
							"   Previous definition of '" << procName << "' was in " << procAtLineNo_[procName] << std::endl;
						return false;
					}
					procAtLineNo_[procName] = file.Pos();

					Procedure proc;
					if (!file.ReadLine()) {
						return false;
					}
					if (!LoadProc(file, procName, leftBracket, proc)) {
						return false;
					}

					if (!file.ReadLine()) {
						break;
					}
					continue;
				}
			}

			{ // try block line
				std::string leftBracket;
				if (PipeFile::IsLeftBracket(file.CurrentLine(), leftBracket)) {
					if (!file.ReadLine()) {
						return false;
					}
					Block block;
					if (!LoadBlock(file, block, (leftBracket == "{{"))) {
						return false;
					}
					size_t blockIndex = blockList_.size();
					blockList_.push_back(block);
					if (!blockList_[0].AppendBlock(blockIndex)) {
						return false;
					}

					if (!file.ReadLine()) {
						break;
					}
					continue;
				}
			}

			{ // try empty line
				if (PipeFile::IsEmptyLine(file.CurrentLine())) {
					if (!file.ReadLine()) {
						break;
					}
					continue;
				}
			}

			{ // try comment line
				if (PipeFile::IsCommentLine(file.CurrentLine())) {
					if (PipeFile::IsDescLine(file.CurrentLine())) {
						if (!PipeFile::ParseAttrLine(file.CurrentLine())) {
							std::cerr << "Warning: Invalid format of attribute at " << file.Pos() << "!" << std::endl;
						}
					}
					if (!file.ReadLine()) {
						break;
					}
					continue;
				}
			}

			{ // try var line
				std::string name;
				std::string value;
				if (PipeFile::IsVarLine(file.CurrentLine(), name, value)) {
					confMap[name] = value;
					if (!file.ReadLine()) {
						break;
					}
					continue;
				}
			}

			{ // try shell command line
				if (!AppendCommandLineFromFile(file, blockList_[0])) {
					return false;
				}
				if (!file.ReadLine()) {
					break;
				}
			}
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
		for (const auto& item : blockList_[it->second.BlockIndex()].GetItems()) {
			file << "\t" << item.ToString() << "\n";
		}
		file << "}\n";
	}

	const Block& block = blockList_[0];
	if (!block.IsEmpty()) {
		if (!procList_.empty()) {
			file << "\n";
		}
		if (block.GetItems().size() == 1) {
			file << block.GetItems()[0].ToString("", *this);
		} else {
			file << block.ToString("", *this);
		}
	}

	file.close();
	return true;
}

void Pipeline::ClearDefaultBlock()
{
	blockList_[0].Clear();
}

void Pipeline::SetDefaultBlock(const std::vector<CommandLineParser>& cmdLineList, bool parallel)
{
	assert(blockList_[0].IsEmpty());
	blockList_[0].SetParallel(parallel);
	for (const auto& cmdLine : cmdLineList) {
		blockList_[0].AppendCommand(cmdLine.GetArgLists(), *this);
	}
}

void Pipeline::SetDefaultBlock(const std::string& cmd, const std::vector<std::string>& arguments)
{
	assert(blockList_[0].IsEmpty());
	blockList_[0].AppendCommand(cmd, arguments);
}

void Pipeline::SetDefaultBlock(const std::string& procName, const ProcArgs& procArgs)
{
	assert(blockList_[0].IsEmpty());
	blockList_[0].AppendCommand(procName, procArgs);
}

bool Pipeline::HasProcedure(const std::string& name) const
{
	return procList_.find(name) != procList_.end();
}

const Block& Pipeline::GetDefaultBlock() const
{
	return blockList_.at(0);
}

const Block& Pipeline::GetBlock(size_t index) const
{
	return blockList_.at(index);
}

const Block& Pipeline::GetBlock(const std::string& procName) const
{
	return blockList_[GetBlockIndex(procName)];
}

size_t Pipeline::GetBlockIndex(const std::string& procName) const
{
	auto it = procList_.find(procName);
	if (it == procList_.end()) {
		throw std::runtime_error("Invalid procName");
	}
	return it->second.BlockIndex();
}

bool Pipeline::HasAnyDefaultCommand() const
{
	return !blockList_[0].IsEmpty();
}

bool CommandItem::ConvertShellToProc()
{
	ProcArgs procArgs;
	for (const auto& arg : shellArgs_) {
		std::smatch sm;
		if (!std::regex_match(arg, sm, std::regex("(\\w+)=(.*)"))) {
			std::cerr << "Error: Invalid option '" << arg << "' for calling '" << shellCmd_ << "'!" << std::endl;
			return false;
		}
		const auto& key = sm[1];
		const auto& value = sm[2];
		if (procArgs.Has(key)) {
			std::cerr << "Error: Duplicated option '" << key << "'!" << std::endl;
			return false;
		}
		procArgs.Add(key, value);
	}
	type_ = CommandType::TYPE_PROC;
	procName_ = shellCmd_;
	procArgs_ = procArgs;
	return true;
}

bool Pipeline::FinalCheckAfterLoad()
{
	auto procNameList = GetProcNameList("");
	std::set<std::string> procNameSet(procNameList.begin(), procNameList.end());
	for (auto& block : blockList_) {
		if (!block.UpdateCommandToProcCalling(procNameSet)) {
			return false;
		}
	}
	return true;
}

void Pipeline::Dump() const
{
	std::cerr << "===== pipeline dump - " << blockList_.size() << " block(s):\n";
	for (size_t i = 0; i < blockList_.size(); ++i) {
		std::cerr << "block[" << i << "]: " << blockList_[i].DetailToString() << "\n";
	}
	std::cerr << "===== Pipeline Dump End =====" << std::endl;
}

size_t Pipeline::AppendBlock(const Block& block)
{
	size_t index = blockList_.size();
	blockList_.push_back(block);
	return index;
}

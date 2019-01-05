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
		s += " " + name + "=" + StringUtils::ShellQuote(it->second, false);
	}
	return s;
}

void ProcArgs::Clear()
{
	args_.clear();
	order_.clear();
}

CommandItem::CommandItem(const std::string& procName, const ProcArgs& procArgs):
	type_(CommandType::TYPE_PROC), procName_(procName), procArgs_(procArgs)
{
	name_ = procName;
}

CommandItem::CommandItem(const std::string& fullCmdLine):
	type_(CommandType::TYPE_SHELL), shellCmd_(fullCmdLine)
{
	name_ = StringUtils::RemoveSpecialCharacters(shellCmd_);
	if (name_.empty()) {
		name_ = "shell";
	}
}

CommandItem::CommandItem(const Block& block):
	type_(CommandType::TYPE_BLOCK), block_(new Block(block))
{
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

std::string CommandItem::ToString(const std::string& indent) const
{
	if (type_ == CommandType::TYPE_SHELL) {
		return indent + ShellCmd() + "\n";
	} else if (Type() == CommandType::TYPE_BLOCK) {
		return GetBlock().ToString(indent, false);
	} else {
		assert(type_ == CommandType::TYPE_PROC);
		return procName_ + procArgs_.ToString() + "\n";
	}
}

void Block::Clear()
{
	items_.clear();
	parallel_ = false;
}

void Block::AppendCommand(const std::string& fullCmdLine)
{
	items_.push_back(CommandItem(fullCmdLine));
}

void Block::AppendCommand(const std::string& procName, const ProcArgs& procArgs)
{
	items_.push_back(CommandItem(procName, procArgs));
}

bool Block::AppendBlock(const Block& block)
{
	items_.push_back(CommandItem(block));
	return true;
}

bool Block::UpdateCommandToProcCalling(const std::set<std::string>& procNameSet)
{
	for (auto& item : items_) {
		if (!item.TryConvertShellToProc(procNameSet)) {
			return false;
		}
	}
	return true;
}

std::string Block::ToString(const std::string& indent, bool root) const
{
	std::string s;
	if (!root || parallel_) {
		s += indent + (parallel_ ? "{{" : "{") + "\n";
	}
	for (const auto& item : items_) {
		s += item.ToString((root && !parallel_) ? indent : indent + "\t");
	}
	if (!root || parallel_) {
		s += indent + (parallel_ ? "}}" : "}") + "\n";
	}
	return s;
}

bool CommandItem::Simplify()
{
	assert(block_ != nullptr);
	return block_->Simplify();
}

bool Block::Simplify(bool root)
{
	bool somethingChanged = false;
	bool requireChecking = true;
	while (requireChecking) {
		requireChecking = false;
		if (items_.size() == 1) {
			if (items_[0].Type() == CommandType::TYPE_BLOCK) {
				if (root || parallel_ == items_[0].GetBlock().parallel_) {
					parallel_ = items_[0].GetBlock().parallel_;
					items_ = items_[0].GetBlock().items_;
					somethingChanged = true;
					requireChecking = true;
				}
			}
		} else {
			for (size_t i = 0; i < items_.size(); ++i) {
				if (items_[i].Type() == CommandType::TYPE_BLOCK) {
					if (items_[i].Simplify()) {
						somethingChanged = true;
						requireChecking = true;
					}
				}
			}
		}
	}
	return somethingChanged;
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
			if (!block.AppendBlock(subBlock)) {
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
	std::string lines = StringUtils::Trim(file.CurrentLine());
	CommandLineParser parser;
	for (;;) {
		if (!parser.Parse(lines)) {
			if (parser.IsUnfinished()) {
				if (!file.ReadLine()) {
					std::cerr << "Unexpected EOF at " << file.Pos() << std::endl;
					return false;
				}
				if (!lines.empty() && lines.substr(lines.size() - 1) == "\\") {
					lines = lines.substr(0, lines.size() - 1);
				} else {
					lines += "\n";
				}
				lines += StringUtils::Trim(file.CurrentLine());
				continue;
			}
			std::cerr << "Error when parsing shell command at " << file.Pos() << ":\n"
				"   " << lines << "\n"
				"   " << parser.ErrorWithLeadingSpaces() << std::endl;
			return false;
		}
		if (!lines.empty()) {
			block.AppendCommand(lines);
		}
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
	procList_[name].Initialize(name, block);
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
					if (!block_.AppendBlock(block)) {
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
				if (!AppendCommandLineFromFile(file, block_)) {
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

	block_.Simplify();
	return true;
}

std::string Procedure::ToString() const
{
	return name_ + "() " + Block::ToString("", false);
}

bool Pipeline::Save(const std::string& filename) const
{
	std::ofstream file(filename);
	if (!file) {
		return false;
	}

	for (auto it = procList_.begin(); it != procList_.end(); ++it) {
		const auto& proc = it->second;
		file << proc.ToString();
	}

	const Block& block = block_;
	if (!block.IsEmpty()) {
		if (!procList_.empty()) {
			file << std::endl;
		}
		file << block.ToString("", true);
	}

	file.close();
	return true;
}

void Pipeline::ClearDefaultBlock()
{
	block_.Clear();
}

void Pipeline::SetDefaultBlock(bool parallel, const std::vector<std::string> shellCmdList)
{
	assert(block_.IsEmpty());
	block_.SetParallel(parallel);
	for (const auto& fullCmdLine : shellCmdList) {
		block_.AppendCommand(StringUtils::Trim(fullCmdLine));
	}
}

void Pipeline::SetDefaultBlock(const std::string& procName, const ProcArgs& procArgs)
{
	assert(block_.IsEmpty());
	block_.AppendCommand(procName, procArgs);
}

bool Pipeline::HasProcedure(const std::string& name) const
{
	return procList_.find(name) != procList_.end();
}

const Block& Pipeline::GetDefaultBlock() const
{
	return block_;
}

const Block& Pipeline::GetBlock(const std::string& procName) const
{
	auto it = procList_.find(procName);
	assert(it != procList_.end());
	return it->second;
}

bool Pipeline::HasAnyProcedure() const
{
	return !procList_.empty();
}

bool Pipeline::HasAnyDefaultCommand() const
{
	return !block_.IsEmpty();
}

bool CommandItem::TryConvertShellToProc(const std::set<std::string>& procNameSet)
{
	if (type_ != CommandType::TYPE_SHELL) {
		return true;
	}

	CommandLineParser parser;
	if (!parser.Parse(shellCmd_)) {
		return true;
	}

	const auto argLists = parser.GetArgLists();
	if (argLists.size() != 1) {
		return true;
	}
	const auto& args = argLists[0];
	if (args.empty()) {
		return true;
	}
	const auto& procName = args[0];
	if (procNameSet.find(procName) == procNameSet.end()) {
		return true;
	}

	ProcArgs procArgs;
	for (size_t i = 1; i < args.size(); ++i) {
		std::smatch sm;
		if (!std::regex_match(args[i], sm, std::regex("(\\w+)=(.*)"))) {
			return true;
		}
		const auto& key = sm[1];
		const auto& value = sm[2];
		if (procArgs.Has(key)) {
			std::cerr << "Warning: Duplicated option '" << key << "'!" << std::endl;
			return false;
		}
		procArgs.Add(key, value);
	}

	type_ = CommandType::TYPE_PROC;
	procName_ = procName;
	procArgs_ = procArgs;
	return true;
}

bool Pipeline::FinalCheckAfterLoad()
{
	auto procNameList = GetProcNameList("");
	std::set<std::string> procNameSet(procNameList.begin(), procNameList.end());
	if (!block_.UpdateCommandToProcCalling(procNameSet)) {
		return false;
	}
	return true;
}

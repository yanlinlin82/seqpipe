#ifndef PIPELINE_H__
#define PIPELINE_H__

#include <string>
#include <vector>
#include <map>
#include "PipeFile.h"

std::string FormatProcCalling(const std::string& procName,
		const std::map<std::string, std::string>& procArgs,
		const std::vector<std::string>& procArgsOrder);

class CommandItem
{
public:
	CommandItem() { }
	CommandItem(const std::string& cmd, const std::vector<std::string>& arguments, const std::string& cmdLine = "");
	CommandItem(const std::string& procName, const std::map<std::string, std::string>& procArgs,
			const std::vector<std::string>& procArgsOrder);

	std::string ToString() const;
public:
	enum Type { TYPE_SHELL, TYPE_PROC };

	Type type_ = TYPE_SHELL;
	std::string name_;

	// members for 'shell command'
	std::string cmdLine_;
	std::string shellCmd_;
	std::vector<std::string> shellArgs_;

	// members for 'procedure'
	std::string procName_;
	std::map<std::string, std::string> procArgs_;
	std::vector<std::string> procArgsOrder_;
};

class Block
{
public:
	void Clear();
	bool AppendCommand(const std::string& line);
	bool AppendCommand(const std::string& cmd, const std::vector<std::string>& arguments);
	bool AppendCommand(const std::string& procName, const std::map<std::string, std::string>& procArgs,
			const std::vector<std::string>& procArgsOrder);
	void SetParallel(bool parallel) { parallel_ = parallel; }

	bool HasAnyCommand() const { return !items_.empty(); }

	std::vector<CommandItem> items_;
	bool parallel_ = false;
};

class Procedure
{
public:
	void Initialize(const std::string& name, size_t blockIndex) { name_ = name; blockIndex_ = blockIndex; }

	std::string Name() const { return name_; }
	size_t BlockIndex() const { return blockIndex_; }
private:
	std::string name_;
	size_t blockIndex_ = 0;
};

class Pipeline
{
public:
	static bool CheckIfPipeFile(const std::string& command);

	bool Load(const std::string& filename);
	void FinalCheckAfterLoad();
	bool Save(const std::string& filename) const;

	bool SetDefaultBlock(const std::vector<std::string>& cmdList, bool parallel);
	bool SetDefaultBlock(const std::string& cmd, const std::vector<std::string>& arguments);
	bool SetDefaultBlock(const std::string& procName, const std::map<std::string, std::string>& procArgs,
			const std::vector<std::string>& procArgsOrder);

	bool HasProcedure(const std::string& name) const;
	bool HasAnyDefaultCommand() const;

	const Block& GetDefaultBlock() const;
	const Block& GetBlock(const std::string& procName) const;
	std::vector<std::string> GetProcNameList(const std::string& pattern) const;
private:
	bool LoadConf(const std::string& filename, std::map<std::string, std::string>& confMap);
	bool LoadProc(PipeFile& file, const std::string& name, std::string leftBracket, Procedure& proc);
	bool LoadBlock(PipeFile& file, Block& block, bool parallel);

	bool ReadLeftBracket(PipeFile& file, std::string& leftBracket);
private:
	std::map<std::string, Procedure> procList_;
	std::vector<Block> blockList_{1}; // the first 'Block' is the default one.
};

#endif

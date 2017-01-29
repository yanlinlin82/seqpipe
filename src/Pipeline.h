#ifndef PIPELINE_H__
#define PIPELINE_H__

#include <string>
#include <vector>
#include <map>
#include "PipeFile.h"

class ProcArgs
{
public:
	std::string ToString() const;
	bool IsEmpty() const { return args_.empty(); }
	bool Has(const std::string& key) const { return (args_.find(key) != args_.end()); }
	void Add(const std::string& key, const std::string& value);
	void Clear();
private:
	std::map<std::string, std::string> args_;
	std::vector<std::string> order_;
};

class CommandItem
{
public:
	enum CommandType { TYPE_SHELL, TYPE_PROC };

	CommandItem() { }
	CommandItem(const std::string& cmd, const std::vector<std::string>& arguments);
	CommandItem(const std::string& procName, const ProcArgs& procArgs);

	bool ConvertShellToProc();

	CommandType Type() const { return type_; }
	const std::string& Name() const { return name_; }

	const std::string& CmdLine() const;
	const std::string& ShellCmd() const;

	const std::string& ProcName() const;
	const ProcArgs& GetProcArgs() const;

	std::string ToString() const;
private:
	CommandType type_ = TYPE_SHELL;
	std::string name_;

	// members for 'shell command'
	std::string fullCmdLine_;
	std::string shellCmd_;
	std::vector<std::string> shellArgs_;

	// members for 'procedure'
	std::string procName_;
	ProcArgs procArgs_;
};

class Block
{
public:
	void Clear();
	bool AppendCommand(const std::string& line);
	bool AppendCommand(const std::string& cmd, const std::vector<std::string>& arguments);
	bool AppendCommand(const std::string& procName, const ProcArgs& procArgs);
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
	bool FinalCheckAfterLoad();
	bool Save(const std::string& filename) const;

	bool SetDefaultBlock(const std::vector<std::string>& cmdList, bool parallel);
	bool SetDefaultBlock(const std::string& cmd, const std::vector<std::string>& arguments);
	bool SetDefaultBlock(const std::string& procName, const ProcArgs& procArgs);

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

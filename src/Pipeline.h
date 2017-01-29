#ifndef PIPELINE_H__
#define PIPELINE_H__

#include <string>
#include <vector>
#include <map>
#include "PipeFile.h"

class CommandItem
{
public:
	std::string name_;
	std::string cmdLine_;
};

class Block
{
public:
	bool AppendCommand(const std::string& line);
	bool AppendCommand(const std::string& cmd, const std::vector<std::string>& arguments);
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
	bool Save(const std::string& filename) const;

	bool SetDefaultBlock(const std::vector<std::string>& cmdList, bool parallel);
	bool SetDefaultBlock(const std::string& cmd, const std::vector<std::string>& arguments);
	bool AppendCommand(const std::string& cmd, const std::vector<std::string>& arguments);

	bool HasProcedure(const std::string& name) const;
	bool HasAnyDefaultCommand() const;

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

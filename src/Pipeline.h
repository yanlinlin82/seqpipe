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
	std::vector<CommandItem> items_;
	bool parallel_ = false;
};

class Procedure
{
public:
	std::string Name() const { return name_; }
	std::string Pos() const { return pos_; }
	Block GetBlock() const { return block_; }

	void SetName(const std::string& name) { name_ = name; }
	void SetParallel(bool parallel) { block_.parallel_ = parallel; }
	bool AppendCommand(const std::string& line);
	bool AppendCommand(const std::string& cmd, const std::vector<std::string>& arguments);
private:
	std::string name_;
	Block block_;
	std::string pos_; // format as "filename(line-no)"
};

class Pipeline
{
public:
	static bool CheckIfPipeFile(const std::string& command);

	bool Load(const std::string& filename);
	bool Save(const std::string& filename) const;

	bool SetDefaultProc(const std::vector<std::string>& cmdList, bool parallel);
	bool AppendCommand(const std::string& cmd, const std::vector<std::string>& arguments);
	bool HasProcedure(const std::string& name) const;

	Block GetBlock(const std::string& procName) const;
	std::vector<std::string> GetProcNameList() const;

	const Procedure* GetProc(const std::string& name) const;
private:
	bool LoadConf(const std::string& filename, std::map<std::string, std::string>& confMap);
	bool LoadProc(PipeFile& file, const std::string& name, std::string leftBracket, Procedure& proc);
private:
	std::map<std::string, Procedure> procList_;
	Procedure defaultProc_;
	std::vector<Block> blockList_;
};

#endif

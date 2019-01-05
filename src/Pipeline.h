#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cassert>
#include "PipeFile.h"
#include "CommandLineParser.h"

class ProcArgs
{
public:
	std::string ToString() const;
	bool IsEmpty() const { return args_.empty(); }
	bool Has(const std::string& key) const { return (args_.find(key) != args_.end()); }
	void Add(const std::string& key, const std::string& value);
	std::string Get(const std::string& key) const;
	void Clear();
private:
	std::map<std::string, std::string> args_;
	std::vector<std::string> order_;
};

class Statement
{
public:
	enum CommandType { TYPE_SHELL, TYPE_PROC, TYPE_BLOCK };

	Statement() { }
	Statement(const std::string& procName, const ProcArgs& procArgs);
	explicit Statement(const std::string& fullCmdLine);

	bool TryConvertShellToProc(const std::set<std::string>& procNameSet);

	std::string ToString(const std::string& indent = "", bool root = false) const;

	// common attributes
	CommandType Type() const { return type_; }
	const std::string& Name() const { return name_; }

	// 'shell command' attributes
	const std::string& ShellCmd() const;

	// 'procedure' attributes
	const std::string& ProcName() const;
	const ProcArgs& GetProcArgs() const;

	// 'block' attributes
	bool Simplify(bool root = true);

public:
	void Clear();

	void SetParallel(bool parallel) { type_ = TYPE_BLOCK; parallel_ = parallel; }
	void AppendCommand(const std::string& fullCmdLine);
	void AppendCommand(const std::string& procName, const ProcArgs& procArgs);

	bool AppendStatement(const Statement& block);
	bool UpdateCommandToProcCalling(const std::set<std::string>& procNameSet);

	bool IsEmpty() const { return items_.empty(); }

	const std::vector<Statement>& GetItems() const { return items_; }
	bool IsParallel() const { return parallel_; }

protected:
	void Set(const Statement& block) { type_ = TYPE_BLOCK; items_ = block.items_; parallel_ = block.parallel_; }

private:
	CommandType type_ = TYPE_SHELL;
	std::string name_;

	// members for 'shell command'
	std::string shellCmd_;

	// members for 'procedure'
	std::string procName_;
	ProcArgs procArgs_;

	// members for 'block'
	bool parallel_ = false;
	std::vector<Statement> items_;
};

class Procedure: public Statement
{
public:
	void Initialize(const std::string& name, const Statement& block) { name_ = name; Statement::Set(block); }

	std::string Name() const { return name_; }

	std::string ToString() const;
private:
	std::string name_;
};

class Pipeline
{
public:
	static bool CheckIfPipeFile(const std::string& command);

	bool Load(const std::string& filename);
	bool FinalCheckAfterLoad();
	bool Save(const std::string& filename) const;

	void ClearDefaultStatement();
	void SetDefaultStatement(bool parallel, const std::vector<std::string> shellCmdList);
	void SetDefaultStatement(const std::string& procName, const ProcArgs& procArgs);

	bool HasProcedure(const std::string& name) const;
	bool HasAnyProcedure() const;
	bool HasAnyDefaultCommand() const;

	const Statement& GetDefaultStatement() const;
	const Statement& GetStatement(const std::string& procName) const;
	std::vector<std::string> GetProcNameList(const std::string& pattern) const;
private:
	bool LoadConf(const std::string& filename, std::map<std::string, std::string>& confMap);
	bool LoadProc(PipeFile& file, const std::string& name, std::string leftBracket, Procedure& proc);
	bool LoadBlock(PipeFile& file, Statement& block, bool parallel);

	bool ReadLeftBracket(PipeFile& file, std::string& leftBracket);

	bool AppendCommandLineFromFile(PipeFile& file, Statement& block);
private:
	std::map<std::string, Procedure> procList_;
	std::map<std::string, std::string> procAtLineNo_;
	Statement block_;
};

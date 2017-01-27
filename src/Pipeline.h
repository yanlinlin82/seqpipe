#ifndef PIPELINE_H__
#define PIPELINE_H__

#include <string>
#include <vector>

class CommandItem
{
public:
	std::string name_;
	std::string cmdLine_;
};

class Pipeline
{
public:
	static bool CheckIfPipeFile(const std::string& command);

	bool Load(const std::string& filename);
	bool Save(const std::string& filename) const;

	bool AppendCommand(const std::string& cmd, const std::vector<std::string>& arguments);
	std::string JoinCommandLine(const std::string& cmd, const std::vector<std::string>& arguments);

	std::vector<CommandItem> GetCommandLines() const { return commandLines_; }
	std::vector<std::string> GetModules() const { return modules_; }
private:
	std::vector<std::string> originPipeline_;
	std::vector<CommandItem> commandLines_;
	std::vector<std::string> modules_;
};

#endif

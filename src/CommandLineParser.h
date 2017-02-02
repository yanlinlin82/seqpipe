#ifndef COMMAND_LINE_PARSER_H__
#define COMMAND_LINE_PARSER_H__

#include <string>
#include <vector>

class CommandLineParser
{
public:
	bool Parse(const std::string& s);

	bool IsError() const { return (status_ == STATUS_ERROR); }
	bool IsUnfinished() const { return (status_ == STATUS_UNFINISHED); }
	std::string ErrorWithLeadingSpaces() const;

	const std::vector<std::vector<std::string>>& GetArgLists() const { return argLists_; }
	std::string ToFullCmdLine() const;
private:
	static int ParseHex(int c);
	static int ParseOct(int c);
	static int ParseHex(int c1, int c2);
	static int ParseOct(int c1, int c2);

	void Dump() const;
private:
	std::vector<std::vector<std::string>> argLists_;

	enum Status { STATUS_OK, STATUS_ERROR, STATUS_UNFINISHED };
	Status status_ = STATUS_OK;

	size_t errorPos_ = 0;
	std::string errorMsg_ = "";
};

#endif

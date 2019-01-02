#pragma once
#include <string>
#include <vector>
#include <iostream>

class CommandLineParser
{
public:
	bool Parse(const std::string& s);

	bool IsError() const { return (status_ == STATUS_ERROR); }
	bool IsUnfinished() const { return (status_ == STATUS_UNFINISHED); }
	std::string ErrorWithLeadingSpaces() const;

	const std::vector<std::vector<std::string>>& GetArgLists() const { return argLists_; }
	std::string ToFullCmdLine() const;

	void Dump(std::ostream& os = std::cout) const;
private:
	bool ParseDoubleQuotedString(const std::string& s, size_t& i, std::string& res);
	bool ParseSingleQuotedString(const std::string& s, size_t& i, std::string& res);
	bool ParseEscapeCharacter(const std::string& s, size_t& i, std::string& res);

	static int ParseHex(int c);
	static int ParseOct(int c);
	static int ParseHex(int c1, int c2);
	static int ParseOct(int c1, int c2);
private:
	std::vector<std::vector<std::string>> argLists_;

	enum Status { STATUS_OK, STATUS_ERROR, STATUS_UNFINISHED };
	Status status_ = STATUS_OK;

	size_t errorPos_ = 0;
	std::string errorMsg_ = "";
};

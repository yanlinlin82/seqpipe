#include <iostream>
#include <regex>
#include <cassert>
#include "PipeFile.h"

bool PipeFile::Open(const std::string& filename)
{
	assert(!file_.is_open());

	file_.open(filename);
	if (!file_.is_open()) {
		return false;
	}
	filename_ = filename;
	return true;
}

std::string PipeFile::Pos() const
{
	assert(file_.is_open());
	return ("line " + std::to_string(lineNo_) + " of file '" + filename_ + "'");
}

bool PipeFile::ReadLine()
{
	assert(file_.is_open());
	if (!std::getline(file_, currentLine_)) {
		return false;
	}
	++lineNo_;
	return true;
}

bool PipeFile::IsEmptyLine(const std::string& s)
{
	return std::regex_match(s, std::regex("\\s*"));
}

bool PipeFile::IsCommentLine(const std::string& s)
{
	return std::regex_search(s, std::regex("^\\s*[#]"));
}

bool PipeFile::IsDescLine(const std::string& s)
{
	return std::regex_search(s, std::regex("^\\s*#\\["));
}

bool PipeFile::ParseAttrLine(const std::string& s)
{
	return std::regex_match(s, std::regex("\\s*#\\[(\\w+\\s+|)(\\s*([\\w\\.]+)=\"[^\"]*\"(\\s+([\\w\\.]+)=\"[^\"]*\")*|)\\s*\\]\\s*"));
}

bool PipeFile::IsIncLine(const std::string& s, std::string& filename)
{
	std::smatch sm;
	if (!std::regex_match(s, sm, std::regex("\\s*(SP_include|source|\\.)\\s+(.*)\\s*"))) {
		return false;
	}
	filename = sm[2];
	return true;
}

bool PipeFile::IsVarLine(const std::string& s, std::string& name, std::string& value)
{
	std::smatch sm;
	if (!std::regex_match(s, sm, std::regex("\\s*([\\w\\.]+)\\s*=(.*)"))) {
		return false;
	}
	name = sm[1];
	value = sm[2];
	return true;
}

bool PipeFile::IsFuncLine(const std::string& s, std::string& name, std::string& leftBracket)
{
	std::smatch sm;
	if (std::regex_match(s, sm, std::regex("\\s*function\\s+([\\w\\.]+)\\s*(\\{|\\{\\{|)\\s*"))) {
		name = sm[1];
		leftBracket = sm[2];
		return true;
	}
	if (std::regex_match(s, sm, std::regex("\\s*([\\w\\.]+)\\s*\\(\\s*\\)\\s*(\\{|\\{\\{|)\\s*"))) {
		name = sm[1];
		leftBracket = sm[2];
		return true;
	}
	return false;
}

bool PipeFile::IsLeftBracket(const std::string& s, std::string& leftBracket)
{
	std::smatch sm;
	if (!std::regex_match(s, sm, std::regex("\\s*({|{{)\\s*"))) {
		return false;
	}
	leftBracket = sm[1];
	return true;
}

bool PipeFile::IsRightBracket(const std::string& s, std::string& rightBracket)
{
	std::smatch sm;
	if (!std::regex_match(s, sm, std::regex("\\s*(}|}})\\s*"))) {
		return false;
	}
	rightBracket = sm[1];
	return true;
}

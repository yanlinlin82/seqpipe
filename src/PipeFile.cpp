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
	return filename_ + "(" + std::to_string(lineNo_) + ")";
}

bool PipeFile::ReadLine()
{
	assert(file_.is_open());
	if (!std::getline(file_, currentLine_)) {
		return false;
	}
	++lineNo_;

	std::cout << Pos() << "\t" << CurrentLine() << std::endl;
	return true;
}

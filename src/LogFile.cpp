#include <iostream>
#include "LogFile.h"

bool LogFile::Initialize(const std::string& filename)
{
	file_.open(filename);
	return file_.is_open();
}

void LogFile::WriteLine(const std::string& s)
{
	std::lock_guard<std::mutex> lock(mutex_);
	std::cout << s << std::endl;
	file_ << s << std::endl;
}

#include <iostream>
#include "LogFile.h"

LogFile::LogFile(const std::string& filename): file_(filename)
{
}

void LogFile::WriteLine(const std::string& s)
{
	std::lock_guard<std::mutex> lock(mutex_);
	std::cout << s << std::endl;
	file_ << s << std::endl;
}

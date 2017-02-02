#include <iostream>
#include <fstream>
#include "SysInfo.h"
#include "System.h"
#include "SeqPipe.h"
#include "TimeString.h"

bool SysInfo::WriteToFile(const std::string& filename)
{
	std::ofstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Error: Can not write to file '" << filename << "'!" << std::endl;
		return false;
	}

	file << "===== System Information =====\n"
		"System: " + System::RunShell("uname -a") +
		"\n"
		"Date: " + TimeString(time(NULL)) + "\n"
		"Pwd : " + System::GetCurrentDirectory() + "\n"
		"\n"
		"CPU:\n" + System::RunShell("lscpu") +
		"\n"
		"Memory:\n" + System::RunShell("free -g") +
		"\n"
		"===== SeqPipe Version =====\n"
		"SeqPipe: " + VERSION + "\n"
		"SeqPipe Path: " + System::GetCurrentExe() + "\n"
		<< std::endl;

	file.close();
	return true;
}

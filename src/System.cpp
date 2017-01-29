#include <iostream>
#include <fstream>
#include <vector>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "System.h"
#include "StringUtils.h"

std::string System::GetHostname()
{
	static char buffer[256] = "";
	if (!buffer[0]) {
		gethostname(buffer, sizeof(buffer));
	}
	return buffer;
}

std::string System::GetFullCommandLine()
{
	static std::string cmdLine;
	if (cmdLine.empty()) {
		auto filename = "/proc/" + std::to_string(getpid()) + "/cmdline";
		std::ifstream file(filename);
		std::string word;
		while (std::getline(file, word, '\0')) {
			if (!cmdLine.empty()) {
				cmdLine += ' ';
			}
			cmdLine += EncodeShell(word);
		}
	}
	return cmdLine;
}

std::string System::GetCurrentExe()
{
	char buffer[256] = "";
	if (readlink("/proc/self/exe", buffer, sizeof(buffer)) < 0) {
		return "";
	}
	return buffer;
}

bool System::CheckFileExists(const std::string& path)
{
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0);
}

bool System::CheckDirectoryExists(const std::string& path)
{
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode));
}

bool System::CreateDirectory(const std::string& path, int mode)
{
	return (mkdir(path.c_str(), mode) == 0);
}

bool System::EnsureDirectory(const std::string& path, int mode)
{
	if (CheckDirectoryExists(path)) {
		return true;
	}
	return CreateDirectory(path, mode);
}

bool System::IsTextFile(const std::string& path)
{
	std::ifstream file(path, std::ifstream::binary);
	if (!file) {
		return false;
	}

	file.seekg(0, file.end);
	int length = file.tellg();
	file.seekg(0, file.beg);

	if (length > 4 * 1024) {
		length = 4 * 1024;
	}
	std::vector<char> buffer;
	buffer.resize(length);

	file.read(&buffer[0], length);
	if (!file) {
		file.close();
		return false;
	}
	for (int i = 0; i < file.gcount(); ++i) {
		int c = buffer[i];
		if (c > 0 && c < 0x20 && c != '\r' && c != '\t' && c != '\n') {
			file.close();
			return false;
		}
	}
	file.close();
	return true;
}

bool System::HasExecutiveAttribute(const std::string& path)
{
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0 && (buffer.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)));
}

std::string System::DirName(const std::string& path)
{
	std::string buffer = path;
	return dirname(&buffer[0]);
}

bool System::IsShellCmd(const std::string& path)
{
	auto fullpath = StringUtils::Trim(RunShell("/usr/bin/which " + path + " 2>/dev/null"));
	return !fullpath.empty() && HasExecutiveAttribute(fullpath);
}

std::string System::EncodeShell(const std::string& s)
{
	if (s.find_first_of(" \t\r\n\'\"$!()[]{};|") != std::string::npos) {
		std::string t;
		for (size_t i = 0; i < s.size(); ++i) {
			if (s[i] == '\'') {
				t += "'\\''";
			} else {
				t += s[i];
			}
		}
		return "'" + t + "'";
	} else {
		return s;
	}
}

int System::Execute(const std::string& cmdLine)
{
	pid_t pid = fork();
	if (pid < 0) {
		return -1;
	} else if (pid == 0) {
		execl("/bin/bash", "bash", "-c", cmdLine.c_str(), NULL);
		exit(1);
	} else {
		int status;
		waitpid(pid, &status, 0);
		if (WIFEXITED(status)) {
			return WEXITSTATUS(status);
		} else {
			return -1;
		}
	}
}

std::string System::RunShell(const std::string& cmdLine)
{
	FILE* fp = popen(cmdLine.c_str(), "r");
	std::string text;
	if (fp) {
		while (!feof(fp)) {
			char buffer[1024];
			size_t n = fread(buffer, 1, sizeof(buffer), fp);
			if (n > 0) {
				text.insert(text.end(), buffer, buffer + n);
			}
		}
	}
	fclose(fp);
	return text;
}

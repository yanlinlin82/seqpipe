#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <regex>
#include <cassert>
#include <libgen.h>
#include <fnmatch.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <dirent.h>
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

std::string System::GetUserName()
{
	static char buffer[64] = "";
	if (!buffer[0]) {
		getlogin_r(buffer, sizeof(buffer));
	}
	return buffer;
}

unsigned int System::GetUserId()
{
	return static_cast<unsigned int>(getuid());
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
			std::smatch sm;
			if (std::regex_match(word, sm, std::regex("(\\w+)=(.*)"))) {
				std::string key = sm[1];
				std::string value = sm[2];
				cmdLine += key + "=" + StringUtils::ShellQuote(value, true);
			} else {
				cmdLine += StringUtils::ShellQuote(word, true);
			}
		}
	}
	return cmdLine;
}

std::string System::GetCurrentDirectory()
{
	char buffer[256] = "";
	return getcwd(buffer, sizeof(buffer));
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

std::vector<std::string> System::ListFiles(const std::string& dir, const std::string& pattern)
{
	std::vector<std::string> files;

	struct dirent** namelist = NULL;
	int n = scandir(dir.c_str(), &namelist, NULL, alphasort);
	if (n >= 0) {
		for (int i = 0; i < n; ++i) {
			if (fnmatch(pattern.c_str(), namelist[i]->d_name, 0) == 0) {
				files.push_back(namelist[i]->d_name);
			}
			free(namelist[i]);
		}
		free(namelist);
	}
	return files;
}

bool System::IsShellCmd(const std::string& path)
{
	auto fullpath = StringUtils::Trim(RunShell("/usr/bin/which " + path + " 2>/dev/null"));
	return !fullpath.empty() && HasExecutiveAttribute(fullpath);
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

std::string System::GetUName()
{
	utsname buf = { };
	if (uname(&buf) < 0) {
		return "-";
	}
	return std::string(buf.sysname) + " " + buf.release + " " + buf.machine;
}

std::string System::GetCPUInfo()
{
	std::map<std::string, size_t> modules;

	std::ifstream file("/proc/cpuinfo");
	if (file.is_open()) {
		std::string line;
		std::regex e("model name\\s*:\\s*(.*)\\s*");
		while (std::getline(file, line)) {
			std::smatch s;
			if (std::regex_match(line, s, e)) {
				std::string name = s[1];
				++modules[name];
			}
		}
		file.close();
	}

	if (modules.empty()) {
		return "-";
	} else {
		std::string s = "";
		for (auto it = modules.begin(); it != modules.end(); ++it) {
			if (!s.empty()) {
				s += " + ";
			}
			s += std::to_string(it->second) + " core(s) (" + it->first + ")";
		}
		return s;
	}
}

std::string System::GetMemoryInfo()
{
	int total = -1;

	std::ifstream file("/proc/meminfo");
	if (file.is_open()) {
		std::string line;
		while (std::getline(file, line)) {
			std::string::size_type pos = line.find_first_of(':');
			if (pos != std::string::npos) {
				std::string name = line.substr(0, pos);
				if (name == "MemTotal") {
					total = std::stoi(line.substr(pos + 1));
					break;
				}
			}
		}
		file.close();
	}

	if (total < 0) {
		return "-";
	} else if (total == 0) {
		return "0";
	} else if (total < 1024 * 10) {
		return std::to_string(total) + " KB";
	} else if (total < 1024 * 1024 * 10) {
		return std::to_string(total / 1024) + " MB";
	} else {
		return std::to_string(total / 1024 / 1024) + " GB";
	}
}

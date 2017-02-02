#ifndef SYSTEM_H__
#define SYSTEM_H__

#include <string>
#include <vector>

class System
{
public:
	static std::string GetHostname();

	static std::string GetFullCommandLine();
	static std::string GetCurrentDirectory();
	static std::string GetCurrentExe();

	static bool CheckFileExists(const std::string& path);
	static bool CheckDirectoryExists(const std::string& path);
	static bool CreateDirectory(const std::string& path, int mode = 0755);
	static bool EnsureDirectory(const std::string& path, int mode = 0755);
	static bool IsTextFile(const std::string& path);
	static bool HasExecutiveAttribute(const std::string& path);

	static std::string DirName(const std::string& path);
	static std::vector<std::string> ListFiles(const std::string& dir, const std::string& pattern);

	static bool IsShellCmd(const std::string& path);
	static std::string EncodeShell(const std::string& s);

	static int Execute(const std::string& cmdLine);
	static std::string RunShell(const std::string& cmdLine);

	static std::string GetUName();
	static std::string GetCPUInfo();
	static std::string GetMemoryInfo();
};

#endif

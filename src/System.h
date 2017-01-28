#ifndef SYSTEM_H__
#define SYSTEM_H__

#include <string>

class System
{
public:
	static std::string GetHostname();

	static std::string GetFullCommandLine();

	static bool CheckFileExists(const std::string& path);
	static bool CheckDirectoryExists(const std::string& path);
	static bool CreateDirectory(const std::string& path, int mode = 0755);
	static bool EnsureDirectory(const std::string& path, int mode = 0755);
	static bool IsTextFile(const std::string& path);
	static bool HasExecutiveAttribute(const std::string& path);

	static std::string DirName(const std::string& path);

	static std::string GetUniqueId();
	static std::string EncodeShell(const std::string& s);

	static int Execute(const std::string& cmdLine);
	static std::string RunShell(const std::string& cmdLine);
};

#endif

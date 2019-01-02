#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <mutex>

class LogFile
{
public:
	bool Initialize(const std::string& filename);

	void WriteLine(const std::string& s);
private:
	std::ofstream file_;
	std::mutex mutex_;
};

class Msg
{
public:
	template <class T>
	Msg& operator << (T x) { ss_ << x; return *this; }

	operator std::string() const { return ss_.str(); }
private:
	std::ostringstream ss_;
};

void WriteStringToFile(const std::string& filename, const std::string& s);

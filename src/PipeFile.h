#ifndef PIPE_FILE_H__
#define PIPE_FILE_H__

#include <fstream>

class PipeFile
{
public:
	bool Open(const std::string& filename);

	bool ReadLine();
	const std::string CurrentLine() const { return currentLine_; }
	std::string Pos() const;

	const std::string& Filename() const { return filename_; }
private:
	std::string filename_;
	std::ifstream file_;
	std::string currentLine_;
	size_t lineNo_ = 0;
};

#endif

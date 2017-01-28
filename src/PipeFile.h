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
public:
	static bool IsEmptyLine(const std::string& s);
	static bool IsCommentLine(const std::string& s);
	static bool IsDescLine(const std::string& s);
	static bool ParseAttrLine(const std::string& s);
	static bool IsIncLine(const std::string& s, std::string& filename);
	static bool IsVarLine(const std::string& s, std::string& name, std::string& value);
	static bool IsFuncLine(const std::string& s, std::string& name, std::string& leftBracket);
	static bool IsLeftBracket(const std::string& s, std::string& leftBracket);
	static bool IsRightBracket(const std::string& s, std::string& rightBracket);
private:
	std::string filename_;
	std::ifstream file_;
	std::string currentLine_;
	size_t lineNo_ = 0;
};

#endif

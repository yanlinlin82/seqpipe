#include <cassert>
#include "CommandLineParser.h"
#include "StringUtils.h"

int CommandLineParser::ParseHex(int c)
{
	if (c >= '0' && c <= '9') {
		return c - '0';
	} else if (c >= 'A' && c <= 'F') {
		return c - 'A' + 10;
	} else if (c >= 'a' && c <= 'f') {
		return c - 'a' + 10;
	} else {
		return -1;
	}
}

int CommandLineParser::ParseOct(int c)
{
	if (c >= '0' && c <= '7') {
		return c - '0';
	} else {
		return -1;
	}
}

int CommandLineParser::ParseHex(int c1, int c2)
{
	int v1 = ParseHex(c1);
	if (v1 < 0) {
		return -1;
	}
	int v2 = ParseHex(c2);
	if (v2 < 0) {
		return -1;
	}
	return v1 * 16 + v2;
}

int CommandLineParser::ParseOct(int c1, int c2)
{
	int v1 = ParseOct(c1);
	if (v1 < 0) {
		return -1;
	}
	int v2 = ParseOct(c2);
	if (v2 < 0) {
		return -1;
	}
	return v1 * 8 + v2;
}

bool CommandLineParser::ParseSingleQuotedString(const std::string& s, size_t& i, std::string& res)
{
	assert(res.empty());

	for (++i; i < s.size(); ++i) {
		if (s[i] == '\'') {
			break;
		}
		res += s[i];
	}
	if (i >= s.size()) {
		status_ = STATUS_UNFINISHED;
		errorPos_ = s.size();
		errorMsg_ = "incomplete single-quoted string";
		return false;
	}
	return true;
}

bool CommandLineParser::ParseDoubleQuotedString(const std::string& s, size_t& i, std::string& res)
{
	assert(res.empty());
	for (++i; i < s.size(); ++i) {
		if (s[i] == '"') {
			break;
		} else if (s[i] == '\\') {
			if (i + 1 >= s.size()) {
				goto incomplete_string;
			}
			++i;
			if (s[i] == 'x') {
				if (i + 2 >= s.size()) {
					goto incomplete_string;
				}
				int value = ParseHex(s[i + 1], s[i + 2]);
				if (value < 0) {
					status_ = STATUS_ERROR;
					errorPos_ = i;
					errorMsg_ = "invalid character for '\\xXX'";
					return false;
				}
				i += 2;
				res += static_cast<char>(value);
			} else if (s[i] == '0') {
				if (i + 2 >= s.size()) {
					goto incomplete_string;
				}
				int value = ParseOct(s[i + 1], s[i + 2]);
				if (value < 0) {
					status_ = STATUS_ERROR;
					errorPos_ = i;
					errorMsg_ = "invalid character for '\\0NN'";
					return false;
				}
				i += 2;
				res += static_cast<char>(value);
			} else if (s[i] == 't' || s[i] == 'r' || s[i] == 'n' || s[i] == 'b') {
				res += '\\';
				res += s[i];
			} else {
				status_ = STATUS_ERROR;
				errorPos_ = i;
				errorMsg_ = "invalid character after '\\'";
				return false;
			}
		} else {
			res += s[i];
		}
	}
	if (i >= s.size()) {
		goto incomplete_string;
	}
	return true;

incomplete_string:
	status_ = STATUS_UNFINISHED;
	errorPos_ = s.size();
	errorMsg_ = "incomplete double-quoted string";
	return false;
}

bool CommandLineParser::ParseEscapeCharacter(const std::string& s, size_t& i, std::string& res)
{
	assert(res.empty());

	if (i + 1 >= s.size()) {
		status_ = STATUS_UNFINISHED;
		errorPos_ = s.size();
		errorMsg_ = "incomplete command line";
		return false;
	}
	++i;

	if (s[i] == '\n') {
		res = "";
	} else if (s[i] == '\r' && i + 1 < s.size() && s[i + 1] == '\n') {
		++i;
		res = "";
	} else {
		res = s[i];
	}
	return true;
}

bool CommandLineParser::Parse(const std::string& s)
{
	argLists_.clear();
	argLists_.resize(1);

	std::string word;
	for (size_t i = 0; i < s.size(); ++i) {
		if (s[i] == ';') {
			if (!word.empty()) {
				argLists_.back().push_back(word);
				word = "";
			}
			argLists_.resize(argLists_.size() + 1);
		} else if (isspace(s[i])) { // space, '\t', '\r', '\n', '\f', '\v'
			if (!word.empty()) {
				argLists_.back().push_back(word);
				word = "";
			}
		} else if (s[i] == '\'') {
			std::string res;
			if (!ParseSingleQuotedString(s, i, res)) {
				return false;
			}
			word += res;
		} else if (s[i] == '"') {
			std::string res;
			if (!ParseDoubleQuotedString(s, i, res)) {
				return false;
			}
			word += res;
		} else if (s[i] == '\\') {
			std::string res;
			if (!ParseEscapeCharacter(s, i, res)) {
				return false;
			}
		} else {
			word += s[i];
		}
	}
	if (!word.empty()) {
		argLists_.back().push_back(word);
	}
	return true;
}

std::string CommandLineParser::ToFullCmdLine() const
{
	std::string s;
	for (size_t i = 0; i < argLists_.size(); ++i) {
		if (i > 0) {
			s += "; ";
		}
		for (size_t j = 0; j < argLists_[i].size(); ++j) {
			if (j > 0) {
				s += " ";
			}
			s += StringUtils::ShellQuote(argLists_[i][j], false);
		}
	}
	return s;
}

void CommandLineParser::Dump(std::ostream& os) const
{
	os << "Total " << argLists_.size() << " args:\n";
	for (size_t i = 0; i < argLists_.size(); ++i) {
		os << "[" << i << "] ";
		for (const auto& x : argLists_[i]) {
			os << StringUtils::ShellQuote(x, false);
		}
		os << "\n";
	}
	os << std::flush;
}

std::string CommandLineParser::ErrorWithLeadingSpaces() const
{
	return std::string(errorPos_, ' ') + "^ " + errorMsg_;
}

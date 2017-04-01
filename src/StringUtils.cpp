#include <iostream>
#include <deque>
#include "StringUtils.h"

const std::string StringUtils::SPACES = " \t\r\n";

std::string StringUtils::TrimLeft(const std::string& s, const std::string& spaces)
{
	std::string::size_type pos = s.find_first_not_of(spaces);
	return (pos == std::string::npos ? "" : s.substr(pos));
}

std::string StringUtils::TrimRight(const std::string& s, const std::string& spaces)
{
	std::string::size_type pos = s.find_last_not_of(spaces);
	return (pos == std::string::npos ? "" : s.substr(0, pos + 1));
}

std::string StringUtils::Trim(const std::string& s, const std::string& spaces)
{
	return TrimLeft(TrimRight(s, spaces), spaces);
}

std::string StringUtils::RemoveSpecialCharacters(const std::string& s)
{
	std::string t;
	for (size_t i = 0; i < s.size(); ++i) {
		if (s[i] == '_' || isalnum(s[i])) {
			t += s[i];
		} else if (s[i] == '/') {
			t = "";
		} else if (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n' || s[i] == ';') {
			if (!t.empty()) {
				break;
			}
		} else if (s[i] == '{' || s[i] == '}' || s[i] == '[' || s[i] == ']' ||
				s[i] == '(' || s[i] == ')' || s[i] == '|' || s[i] == '&') {
			continue;
		} else {
			if (t.empty() || t.substr(t.size() - 1) != "_") {
				t += "_";
			}
		}
	}
	return t;
}

std::vector<std::string> StringUtils::Split(const std::string& s, const std::string& sep)
{
	std::vector<std::string> parts;
	for (std::string::size_type start = 0; start < s.size(); ) {
		std::string::size_type pos = s.find(sep, start);
		if (pos == std::string::npos) {
			parts.push_back(s.substr(start));
			break;
		}
		parts.push_back(s.substr(start, pos - start));
		start = pos + sep.size();
	}
	return parts;
}

std::vector<std::string> StringUtils::Split(const std::string& s, const std::regex& sep)
{
	std::string text = s;
	std::vector<std::string> parts;
	std::smatch sm;
	while (std::regex_search(text, sm, sep)) {
		parts.push_back(sm.prefix());
		parts.push_back(sm[1]);
		text = sm.suffix();
	}
	parts.push_back(text);
	return parts;
}

std::string StringUtils::SingleQuote(const std::string& s)
{
	std::string t = "'";
	for (size_t i = 0; i < s.size(); ++i) {
		if (s[i] == '\'') {
			if (t.back() == '\'') {
				t = t.substr(0, t.size() - 1) + "\\''";
			} else {
				t += "'\\''";
			}
		} else {
			t += s[i];
		}
	}
	if (t.back() == '\'') {
		return t.substr(0, t.size() - 1);
	} else {
		return t + "'";
	}
}

std::string StringUtils::DoubleQuote(const std::string& s)
{
	std::string t;
	for (size_t i = 0; i < s.size(); ++i) {
		if (s[i] == '\a') { // alert bell
			t += "\\a";
		} else if (s[i] == '\b') { // backspace
			t += "\\b";
		} else if (s[i] == '\f') { // form feed - new page
			t += "\\f";
		} else if (s[i] == '\n') { // new line
			t += "\\n";
		} else if (s[i] == '\r') { // carriage return
			t += "\\r";
		} else if (s[i] == '\t') { // horizontal tab
			t += "\\t";
		} else if (s[i] == '\v') { // vertical tab
			t += "\\v";
		} else if (s[i] == '\\') { // backslash
			t += "\\\\";
		} else if (s[i] == '\'') { // single quote
			t += "'"; // simplify "\'" to "'"
		} else if (s[i] == '"') { // double quote
			t += "\\\"";
		} else {
			unsigned char x = static_cast<unsigned char>(s[i]);
			if (x < 32) {
				char buf[16] = "";
				snprintf(buf, sizeof(buf), "\\%03o", x);
				t += buf;
			} else if (x >= 127) {
				char buf[16] = "";
				snprintf(buf, sizeof(buf), "\\x%02X", x);
				t += buf;
			} else {
				t += s[i];
			}
		}
	}
	return '"' + t + '"';
}

std::string StringUtils::ShellQuote(const std::string& s, bool singleQuote)
{
	if (s.find_first_of(" \t\r\n\'\"`~!@#$%^&*()[]{};|") == std::string::npos) {
		return s;
	} else if (singleQuote) {
		return SingleQuote(s);
	} else {
		return DoubleQuote(s);
	}
}

#include "UnitTest.h"
#include "../../src/StringUtils.h"

UNIT_TEST(StringUtils, TrimLeft)
{
	UNIT_ASSERT(StringUtils::TrimLeft("abc"  ) == "abc");
	UNIT_ASSERT(StringUtils::TrimLeft(" abc" ) == "abc");
	UNIT_ASSERT(StringUtils::TrimLeft("  abc") == "abc");

	UNIT_ASSERT(StringUtils::TrimLeft("abc "  ) == "abc ");
	UNIT_ASSERT(StringUtils::TrimLeft(" abc " ) == "abc ");
	UNIT_ASSERT(StringUtils::TrimLeft("  abc ") == "abc ");

	UNIT_ASSERT(StringUtils::TrimLeft("a b "  ) == "a b ");
	UNIT_ASSERT(StringUtils::TrimLeft(" a b " ) == "a b ");
	UNIT_ASSERT(StringUtils::TrimLeft("  a b ") == "a b ");

	UNIT_ASSERT(StringUtils::TrimLeft(""  ) == "");
	UNIT_ASSERT(StringUtils::TrimLeft(" " ) == "");
	UNIT_ASSERT(StringUtils::TrimLeft("  ") == "");

	UNIT_ASSERT(StringUtils::TrimLeft("\t\r\n  abc \t\r\n ") == "abc \t\r\n ");
}

UNIT_TEST(StringUtils, TrimRight)
{
	UNIT_ASSERT(StringUtils::TrimRight("abc"  ) == "abc");
	UNIT_ASSERT(StringUtils::TrimRight("abc " ) == "abc");
	UNIT_ASSERT(StringUtils::TrimRight("abc  ") == "abc");

	UNIT_ASSERT(StringUtils::TrimRight(" abc"  ) == " abc");
	UNIT_ASSERT(StringUtils::TrimRight(" abc " ) == " abc");
	UNIT_ASSERT(StringUtils::TrimRight(" abc  ") == " abc");

	UNIT_ASSERT(StringUtils::TrimRight(" a b"  ) == " a b");
	UNIT_ASSERT(StringUtils::TrimRight(" a b " ) == " a b");
	UNIT_ASSERT(StringUtils::TrimRight(" a b  ") == " a b");

	UNIT_ASSERT(StringUtils::TrimRight(""  ) == "");
	UNIT_ASSERT(StringUtils::TrimRight(" " ) == "");
	UNIT_ASSERT(StringUtils::TrimRight("  ") == "");

	UNIT_ASSERT(StringUtils::TrimRight("\t\r\n  abc \t\r\n ") == "\t\r\n  abc");
}

UNIT_TEST(StringUtils, Trim)
{
	UNIT_ASSERT(StringUtils::Trim("abc"  ) == "abc");
	UNIT_ASSERT(StringUtils::Trim(" abc" ) == "abc");
	UNIT_ASSERT(StringUtils::Trim("  abc") == "abc");

	UNIT_ASSERT(StringUtils::Trim("abc "  ) == "abc");
	UNIT_ASSERT(StringUtils::Trim(" abc " ) == "abc");
	UNIT_ASSERT(StringUtils::Trim("  abc ") == "abc");

	UNIT_ASSERT(StringUtils::Trim("a b "  ) == "a b");
	UNIT_ASSERT(StringUtils::Trim(" a b " ) == "a b");
	UNIT_ASSERT(StringUtils::Trim("  a b ") == "a b");

	UNIT_ASSERT(StringUtils::Trim(""  ) == "");
	UNIT_ASSERT(StringUtils::Trim(" " ) == "");
	UNIT_ASSERT(StringUtils::Trim("  ") == "");

	UNIT_ASSERT(StringUtils::Trim("\t\r\n  abc \t\r\n ") == "abc");
}

UNIT_TEST(StringUtils, RemoveSpecialCharacters)
{
	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("abc"     ) == "abc");
	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("abc123"  ) == "abc123");
	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("abc!123" ) == "abc123");
	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("abc#@123") == "abc123");

	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("  abc  "     ) == "abc");
	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("  abc123  "  ) == "abc123");
	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("  abc!123  " ) == "abc123");
	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("  abc#@123  ") == "abc123");

	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("abc 123"  ) == "abc");
	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("  abc 123") == "abc");

	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("( abc )"     ) == "abc");
	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("( abc123 )"  ) == "abc123");
	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("( abc!123 )" ) == "abc123");
	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("( abc#@123 )") == "abc123");

	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("(   abc  )"     ) == "abc");
	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("(   abc123  )"  ) == "abc123");
	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("(   abc!123  )" ) == "abc123");
	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("(   abc#@123  )") == "abc123");

	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("( abc 123 )"  ) == "abc");
	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("(   abc 123 )") == "abc");

	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters(""     ) == "");
	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("!#$"  ) == "");
	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("##123") == "123");
}

UNIT_TEST(StringUtils, SingleQuote)
{
	UNIT_ASSERT(StringUtils::SingleQuote("abc"     ) == "'abc'");
	UNIT_ASSERT(StringUtils::SingleQuote("abc!123" ) == "'abc!123'");
	UNIT_ASSERT(StringUtils::SingleQuote("abc#@123") == "'abc#@123'");
	UNIT_ASSERT(StringUtils::SingleQuote("abc 123" ) == "'abc 123'");

	UNIT_ASSERT(StringUtils::SingleQuote("abc\n123"    ) == "'abc\n123'");
	UNIT_ASSERT(StringUtils::SingleQuote("abc\\n123"   ) == "'abc\\n123'");
	UNIT_ASSERT(StringUtils::SingleQuote("abc\\\n123"  ) == "'abc\\\n123'");
	UNIT_ASSERT(StringUtils::SingleQuote("\tabc\\\n123") == "'\tabc\\\n123'");

	UNIT_ASSERT(StringUtils::SingleQuote(" "   ) == "' '");
	UNIT_ASSERT(StringUtils::SingleQuote("  "  ) == "'  '");
	UNIT_ASSERT(StringUtils::SingleQuote(" \t ") == "' \t '");

	UNIT_ASSERT(StringUtils::SingleQuote("\001") == "'\001'");
	UNIT_ASSERT(StringUtils::SingleQuote("\x7F") == "'\x7F'");
	UNIT_ASSERT(StringUtils::SingleQuote("\x80") == "'\x80'");
	UNIT_ASSERT(StringUtils::SingleQuote("\xFF") == "'\xFF'");

	UNIT_ASSERT(StringUtils::SingleQuote("abc\"123") == "'abc\"123'");
	UNIT_ASSERT(StringUtils::SingleQuote("abc\'123") == "'abc'\\''123'");

	UNIT_ASSERT(StringUtils::SingleQuote("'"     ) == "\\'");
	UNIT_ASSERT(StringUtils::SingleQuote("''"    ) == "\\'\\'");
	UNIT_ASSERT(StringUtils::SingleQuote("'''"   ) == "\\'\\'\\'");
	UNIT_ASSERT(StringUtils::SingleQuote("\""    ) == "'\"'");
	UNIT_ASSERT(StringUtils::SingleQuote("\"\""  ) == "'\"\"'");
	UNIT_ASSERT(StringUtils::SingleQuote("\"\"\"") == "'\"\"\"'");
	UNIT_ASSERT(StringUtils::SingleQuote("'\""   ) == "\\''\"'");
	UNIT_ASSERT(StringUtils::SingleQuote("\"'"   ) == "'\"'\\'");
	UNIT_ASSERT(StringUtils::SingleQuote("'\"'"  ) == "\\''\"'\\'");
	UNIT_ASSERT(StringUtils::SingleQuote("\"'\"" ) == "'\"'\\''\"'");
}

UNIT_TEST(StringUtils, DoubleQuote)
{
	UNIT_ASSERT(StringUtils::DoubleQuote("abc"     ) == "\"abc\"");
	UNIT_ASSERT(StringUtils::DoubleQuote("abc!123" ) == "\"abc!123\"");
	UNIT_ASSERT(StringUtils::DoubleQuote("abc#@123") == "\"abc#@123\"");
	UNIT_ASSERT(StringUtils::DoubleQuote("abc 123" ) == "\"abc 123\"");

	UNIT_ASSERT(StringUtils::DoubleQuote("abc\n123"    ) == "\"abc\\n123\"");
	UNIT_ASSERT(StringUtils::DoubleQuote("abc\\n123"   ) == "\"abc\\\\n123\"");
	UNIT_ASSERT(StringUtils::DoubleQuote("abc\\\n123"  ) == "\"abc\\\\\\n123\"");
	UNIT_ASSERT(StringUtils::DoubleQuote("\tabc\\\n123") == "\"\\tabc\\\\\\n123\"");

	UNIT_ASSERT(StringUtils::DoubleQuote(" "   ) == "\" \"");
	UNIT_ASSERT(StringUtils::DoubleQuote("  "  ) == "\"  \"");
	UNIT_ASSERT(StringUtils::DoubleQuote(" \t ") == "\" \\t \"");

	UNIT_ASSERT(StringUtils::DoubleQuote("\001") == "\"\\001\"");
	UNIT_ASSERT(StringUtils::DoubleQuote("\x7F") == "\"\\x7F\"");
	UNIT_ASSERT(StringUtils::DoubleQuote("\x80") == "\"\\x80\"");
	UNIT_ASSERT(StringUtils::DoubleQuote("\xFF") == "\"\\xFF\"");

	UNIT_ASSERT(StringUtils::DoubleQuote("abc\"123") == "\"abc\\\"123\"");
	UNIT_ASSERT(StringUtils::DoubleQuote("abc\'123") == "\"abc'123\"");

	UNIT_ASSERT(StringUtils::DoubleQuote("'"     ) == "\"'\"");
	UNIT_ASSERT(StringUtils::DoubleQuote("''"    ) == "\"''\"");
	UNIT_ASSERT(StringUtils::DoubleQuote("'''"   ) == "\"'''\"");
	UNIT_ASSERT(StringUtils::DoubleQuote("\""    ) == "\"\\\"\"");
	UNIT_ASSERT(StringUtils::DoubleQuote("\"\""  ) == "\"\\\"\\\"\"");
	UNIT_ASSERT(StringUtils::DoubleQuote("\"\"\"") == "\"\\\"\\\"\\\"\"");
	UNIT_ASSERT(StringUtils::DoubleQuote("'\""   ) == "\"'\\\"\"");
	UNIT_ASSERT(StringUtils::DoubleQuote("\"'"   ) == "\"\\\"'\"");
	UNIT_ASSERT(StringUtils::DoubleQuote("'\"'"  ) == "\"'\\\"'\"");
	UNIT_ASSERT(StringUtils::DoubleQuote("\"'\"" ) == "\"\\\"'\\\"\"");
}

UNIT_TEST(StringUtils, ShellQuote_WithSingleQuote)
{
	UNIT_ASSERT(StringUtils::ShellQuote("abc",      true) == "abc");
	UNIT_ASSERT(StringUtils::ShellQuote("a#bc",     true) == "'a#bc'");
	UNIT_ASSERT(StringUtils::ShellQuote("a#b@c123", true) == "'a#b@c123'");

	UNIT_ASSERT(StringUtils::ShellQuote("a b",       true) == "'a b'");
	UNIT_ASSERT(StringUtils::ShellQuote("a \r\n\tb", true) == "'a \r\n\tb'");

	UNIT_ASSERT(StringUtils::ShellQuote("${FOO}",        true) == "'${FOO}'");
	UNIT_ASSERT(StringUtils::ShellQuote("X${FOO}Y",      true) == "'X${FOO}Y'");
	UNIT_ASSERT(StringUtils::ShellQuote("\t ${FOO}\r\n", true) == "'\t ${FOO}\r\n'");
	UNIT_ASSERT(StringUtils::ShellQuote("\"${FOO}\"",    true) == "'\"${FOO}\"'");
	UNIT_ASSERT(StringUtils::ShellQuote("'${FOO}'",      true) == "\\''${FOO}'\\'");
}

UNIT_TEST(StringUtils, ShellQuote_WithDoubleQuote)
{
	UNIT_ASSERT(StringUtils::ShellQuote("abc",      false) == "abc");
	UNIT_ASSERT(StringUtils::ShellQuote("a#bc",     false) == "\"a#bc\"");
	UNIT_ASSERT(StringUtils::ShellQuote("a#b@c123", false) == "\"a#b@c123\"");

	UNIT_ASSERT(StringUtils::ShellQuote("a b",       false) == "\"a b\"");
	UNIT_ASSERT(StringUtils::ShellQuote("a \r\n\tb", false) == "\"a \\r\\n\\tb\"");

	UNIT_ASSERT(StringUtils::ShellQuote("${FOO}",        false) == "\"${FOO}\"");
	UNIT_ASSERT(StringUtils::ShellQuote("X${FOO}Y",      false) == "\"X${FOO}Y\"");
	UNIT_ASSERT(StringUtils::ShellQuote("\t ${FOO}\r\n", false) == "\"\\t ${FOO}\\r\\n\"");
	UNIT_ASSERT(StringUtils::ShellQuote("\"${FOO}\"",    false) == "\"\\\"${FOO}\\\"\"");
	UNIT_ASSERT(StringUtils::ShellQuote("'${FOO}'",      false) == "\"'${FOO}'\"");
}

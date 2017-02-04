#include "UnitTest.h"
#include "../../src/StringUtils.h"

UNIT_TEST(StringUtils, Trim)
{
	UNIT_ASSERT(StringUtils::TrimLeft ("abc") == "abc");
	UNIT_ASSERT(StringUtils::TrimRight("abc") == "abc");
	UNIT_ASSERT(StringUtils::Trim     ("abc") == "abc");

	UNIT_ASSERT(StringUtils::TrimLeft (" abc") == "abc");
	UNIT_ASSERT(StringUtils::TrimRight(" abc") == " abc");
	UNIT_ASSERT(StringUtils::Trim     (" abc") == "abc");

	UNIT_ASSERT(StringUtils::TrimLeft ("abc ") == "abc ");
	UNIT_ASSERT(StringUtils::TrimRight("abc ") == "abc");
	UNIT_ASSERT(StringUtils::Trim     ("abc ") == "abc");

	UNIT_ASSERT(StringUtils::TrimLeft (" abc ") == "abc ");
	UNIT_ASSERT(StringUtils::TrimRight(" abc ") == " abc");
	UNIT_ASSERT(StringUtils::Trim     (" abc ") == "abc");
}

UNIT_TEST(StringUtils, RemoveSpecialCharacters)
{
	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("abc") == "abc");
	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("abc#@123") == "abc123");
	UNIT_ASSERT(StringUtils::RemoveSpecialCharacters("abc 123") == "abc");
}

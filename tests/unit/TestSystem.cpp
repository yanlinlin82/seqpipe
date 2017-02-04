#include "UnitTest.h"
#include "../../src/System.h"

UNIT_TEST(TestSystem, EncodeShell)
{
	UNIT_ASSERT(System::EncodeShell("abc", true) == "abc");
	UNIT_ASSERT(System::EncodeShell("a#bc", true) == "a#bc");
	UNIT_ASSERT(System::EncodeShell("a#b@c123", true) == "a#b@c123");

	UNIT_ASSERT(System::EncodeShell("a b", true) == "'a b'");

	UNIT_ASSERT(System::EncodeShell("${FOO}", false) == "${FOO}");
	UNIT_ASSERT(System::EncodeShell("\"${FOO}\"", false) == "\\\"${FOO}\\\"");

	UNIT_ASSERT(System::EncodeShell("${FOO}", true) == "'${FOO}'");
	UNIT_ASSERT(System::EncodeShell("\"${FOO}\"", true) == "'\"${FOO}\"'");
}

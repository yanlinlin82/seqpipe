#include <unistd.h>
#include "UnitTest.h"
#include "../../src/CommandLineParser.h"

UNIT_TEST(CommandLineParser, Simple)
{
	CommandLineParser parser;
	UNIT_ASSERT(parser.Parse("echo hello") != false);

	const auto& argsList = parser.GetArgLists();
	UNIT_ASSERT(argsList.size() == 1);
	UNIT_ASSERT(argsList[0].size() == 2);
	UNIT_ASSERT(argsList[0][0] == "echo");
	UNIT_ASSERT(argsList[0][1] == "hello");
}

UNIT_TEST(CommandLineParser, HasQuote)
{
	CommandLineParser parser;
	UNIT_ASSERT(parser.Parse("echo \"hello\"") != false);

	const auto& argsList = parser.GetArgLists();
	UNIT_ASSERT(argsList.size() == 1);
	UNIT_ASSERT(argsList[0].size() == 2);
	UNIT_ASSERT(argsList[0][0] == "echo");
	UNIT_ASSERT(argsList[0][1] == "hello");
}

UNIT_TEST(CommandLineParser, HasQuoteAndSpace)
{
	CommandLineParser parser;
	UNIT_ASSERT(parser.Parse("echo \"hello world\"") != false);

	const auto& argsList = parser.GetArgLists();
	UNIT_ASSERT(argsList.size() == 1);
	UNIT_ASSERT(argsList[0].size() == 2);
	UNIT_ASSERT(argsList[0][0] == "echo");
	UNIT_ASSERT(argsList[0][1] == "hello world");
}

UNIT_TEST(CommandLineParser, MulitLine)
{
	CommandLineParser parser;
	UNIT_ASSERT(parser.Parse("abc\\\ndef") != false);

	const auto& argsList = parser.GetArgLists();
	UNIT_ASSERT(argsList.size() == 1);
	UNIT_ASSERT(argsList[0].size() == 1);
	UNIT_ASSERT(argsList[0][0] == "abcdef");
}

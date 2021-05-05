/*This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
 Copyright Utrecht University(Department of Information and Computing Sciences)*/

#include "Utility.h"
#include <gtest/gtest.h>

TEST(CheckSafeStoi, CorrectInput)
{
	std::string input = "123";

	int output = Utility::safeStoi(input);
	ASSERT_EQ(output, 123);
	ASSERT_EQ(errno, 0);
}

TEST(CheckSafeStoi, LongSizeInput)
{
	std::string input = "9876543210";

	int output = Utility::safeStoi(input);
	ASSERT_EQ(output, 0);
	ASSERT_EQ(errno, ERANGE);
}

TEST(CheckSafeStoi, PartiallyParseable)
{
	std::string input = "123Hallo";

	int output = Utility::safeStoi(input);
	ASSERT_EQ(output, 123);
	ASSERT_EQ(errno, 0);
}

TEST(CheckSafeStoi, NonParseable)
{
	std::string input = "Hallo123";

	int output = Utility::safeStoi(input);
	ASSERT_EQ(output, 0);
	ASSERT_EQ(errno, EDOM);
}

TEST(CheckSafeStoll, CorrectInput)
{
	std::string input = "9876543210";

	long long output = Utility::safeStoll(input);
	ASSERT_EQ(output, 9876543210);
	ASSERT_EQ(errno, 0);
}

TEST(CheckSafeStoll, OutOfRange)
{
	std::string input = "123456789012345678901234567890";

	long long output = Utility::safeStoll(input);
	ASSERT_EQ(output, 0);
	ASSERT_EQ(errno, ERANGE);
}

TEST(CheckSafeStoll, PartiallyParseable)
{
	std::string input = "9876543210Bonjour";

	long long output = Utility::safeStoll(input);
	ASSERT_EQ(output, 9876543210);
	ASSERT_EQ(errno, 0);
}

TEST(CheckSafeStoll, NonParseable)
{
	std::string input = "Bonjour9876543210";

	long long output = Utility::safeStoll(input);
	ASSERT_EQ(output, 0);
	ASSERT_EQ(errno, EDOM);
}

TEST(CheckAppendBy, SmallTest)
{
	std::vector<char> input = {'a', 'b', 'c', 'd'};
	std::string word 	= "nice";
	char delimiter 		= '\n';
	std::vector<char> output = input;
	const std::string expected = "abcdnice\n";

	Utility::appendBy(output, word, delimiter);
	ASSERT_EQ(output.size(), 9);
	for (int i = 0; i < output.size(); i++)
	{
		ASSERT_EQ(output[i], expected[i]);
	}
}

TEST(CheckStringSplit, NoSplit)
{
	std::string input = "1234567890987654321234567890987654321234567890987654321234567890";

	std::vector<std::string> output = Utility::splitStringOn(input, ' ');
	ASSERT_EQ(output.size(), 1);
	ASSERT_EQ(output[0], "1234567890987654321234567890987654321234567890987654321234567890");
}

TEST(CheckStringSplit, SingleSplit)
{
	std::string input = "line1\nline2";

	std::vector<std::string> output = Utility::splitStringOn(input, '\n');
	ASSERT_EQ(output.size(), 2);
	ASSERT_EQ(output[0], "line1");
	ASSERT_EQ(output[1], "line2");
}

TEST(CheckStringSplit, BackToBackSplit)
{
	std::string input = "line1\n\nline2";

	std::vector<std::string> output = Utility::splitStringOn(input, '\n');
	ASSERT_EQ(output.size(), 3);
	ASSERT_EQ(output[0], "line1");
	ASSERT_EQ(output[1], "");
	ASSERT_EQ(output[2], "line2");
}

TEST(CheckStringSplit, FrontSplit)
{
	std::string input = "\nline2";

	std::vector<std::string> output = Utility::splitStringOn(input, '\n');
	ASSERT_EQ(output.size(), 2);
	ASSERT_EQ(output[0], "");
	ASSERT_EQ(output[1], "line2");
}

TEST(CheckStringSplit, EndSplit)
{
	std::string input = "line1\n";

	std::vector<std::string> output = Utility::splitStringOn(input, '\n');
	ASSERT_EQ(output.size(), 1);
	ASSERT_EQ(output[0], "line1");
}

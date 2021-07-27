/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "Utility.h"

#include <gtest/gtest.h>

// Checks if parseable string is converted correctly by safeStoi.
TEST(CheckSafeStoi, CorrectInput)
{
	std::string input = "123";

	int output = Utility::safeStoi(input);
	ASSERT_EQ(output, 123);
	ASSERT_EQ(errno, 0);
}

// Checks if input that is bigger than int range gives the correct response for safeStoi.
TEST(CheckSafeStoi, LongSizeInput)
{
	std::string input = "9876543210";

	int output = Utility::safeStoi(input);
	ASSERT_EQ(output, 0);
	ASSERT_EQ(errno, ERANGE);
}

// Checks if the partial parsing functions as desired for safeStoi.
TEST(CheckSafeStoi, PartiallyParseable)
{
	std::string input = "123Hallo";

	int output = Utility::safeStoi(input);
	ASSERT_EQ(output, 123);
	ASSERT_EQ(errno, 0);
}

// Checks if non-parseable input gives the correct response for safeStoi.
TEST(CheckSafeStoi, NonParseable)
{
	std::string input = "Hallo123";

	int output = Utility::safeStoi(input);
	ASSERT_EQ(output, 0);
	ASSERT_EQ(errno, EDOM);
}

// Checks if parseable string is converted correctly by safeStoll.
TEST(CheckSafeStoll, CorrectInput)
{
	std::string input = "9876543210";

	long long output = Utility::safeStoll(input);
	ASSERT_EQ(output, 9876543210);
	ASSERT_EQ(errno, 0);
}

// Checks if input that is bigger than long long range gives the correct response for safeStoll.
TEST(CheckSafeStoll, OutOfRange)
{
	std::string input = "123456789012345678901234567890";

	long long output = Utility::safeStoll(input);
	ASSERT_EQ(output, 0);
	ASSERT_EQ(errno, ERANGE);
}

// Checks if the partial parsing functions as desired for safeStoll.
TEST(CheckSafeStoll, PartiallyParseable)
{
	std::string input = "9876543210Bonjour";

	long long output = Utility::safeStoll(input);
	ASSERT_EQ(output, 9876543210);
	ASSERT_EQ(errno, 0);
}

// Checks if non-parseable input gives the correct response for safeStoll.
TEST(CheckSafeStoll, NonParseable)
{
	std::string input = "Bonjour9876543210";

	long long output = Utility::safeStoll(input);
	ASSERT_EQ(output, 0);
	ASSERT_EQ(errno, EDOM);
}

// Checks if parseable string is converted correctly by safeStod.
TEST(CheckSafeStod, CorrectInput)
{
	std::string input = "9876543210";

	long long output = Utility::safeStod(input);
	ASSERT_EQ(output, 9876543210);
	ASSERT_EQ(errno, 0);
}

// Checks if the partial parsing functions as desired for safeStod.
TEST(CheckSafeStod, PartiallyParseable)
{
	std::string input = "9876543210Bonjour";

	long long output = Utility::safeStod(input);
	ASSERT_EQ(output, 9876543210);
	ASSERT_EQ(errno, 0);
}

// Checks if non-parseable input gives the correct response for safeStod.
TEST(CheckSafeStod, NonParseable)
{
	std::string input = "Bonjour9876543210";

	long long output = Utility::safeStod(input);
	ASSERT_EQ(output, 0);
	ASSERT_EQ(errno, EDOM);
}

// Checks if appendBy functions as desired.
TEST(CheckAppendBySingleWord, SmallTest)
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

// Checks if appendBy (with vector of words) works if vector of words is empty.
TEST(CheckAppendByMultipleWords, EmptyVector)
{
	std::vector<char> input = {'a', 'b', 'c', 'd'};
	std::vector<std::string> words = {};
	char wordSeparator = ' ';
	char endCharacter = '\n';
	std::vector<char> output = input;
	const std::string expected = "abcd\n";

	Utility::appendBy(output, words, wordSeparator, endCharacter);
	ASSERT_EQ(output.size(), 5);
	for (int i = 0; i < output.size(); i++)
	{
		ASSERT_EQ(output[i], expected[i]);
	}
}

// Checks if appendBy (with vector of words) works with multiple words.
TEST(CheckAppendByMultipleWords, SmallTest)
{
	std::vector<char> input = {'T', 'h', 'i', 's', ' '};
	std::vector<std::string> words = {"is", "a", "test"};
	char wordSeparator = ' ';
	char endCharacter = '.';
	std::vector<char> output = input;
	const std::string expected = "This is a test.";

	Utility::appendBy(output, words, wordSeparator, endCharacter);
	ASSERT_EQ(output.size(), 15);
	for (int i = 0; i < output.size(); i++)
	{
		ASSERT_EQ(output[i], expected[i]);
	}
}

// Checks if splitStringOn works if no delimiter is present.
TEST(CheckStringSplit, NoSplit)
{
	std::string input = "1234567890987654321234567890987654321234567890987654321234567890";

	std::vector<std::string> output = Utility::splitStringOn(input, ' ');
	ASSERT_EQ(output.size(), 1);
	ASSERT_EQ(output[0], "1234567890987654321234567890987654321234567890987654321234567890");
}

// Checks if splitStringOn works with a single delimiter.
TEST(CheckStringSplit, SingleSplit)
{
	std::string input = "line1\nline2";

	std::vector<std::string> output = Utility::splitStringOn(input, '\n');
	ASSERT_EQ(output.size(), 2);
	ASSERT_EQ(output[0], "line1");
	ASSERT_EQ(output[1], "line2");
}

// Checks if splitStringOn works with back-to-back delimiters.
TEST(CheckStringSplit, BackToBackSplit)
{
	std::string input = "line1\n\nline2";

	std::vector<std::string> output = Utility::splitStringOn(input, '\n');
	ASSERT_EQ(output.size(), 3);
	ASSERT_EQ(output[0], "line1");
	ASSERT_EQ(output[1], "");
	ASSERT_EQ(output[2], "line2");
}

// Checks if splitStringOn works with a delimiter at the front of the input.
TEST(CheckStringSplit, FrontSplit)
{
	std::string input = "\nline2";

	std::vector<std::string> output = Utility::splitStringOn(input, '\n');
	ASSERT_EQ(output.size(), 2);
	ASSERT_EQ(output[0], "");
	ASSERT_EQ(output[1], "line2");
}

// Checks if splitStringOn ignores a delimiter at the end of the input.
TEST(CheckStringSplit, EndSplit)
{
	std::string input = "line1\n";

	std::vector<std::string> output = Utility::splitStringOn(input, '\n');
	ASSERT_EQ(output.size(), 1);
	ASSERT_EQ(output[0], "line1");
}

// Checks if hashToUUIDString correctly changes a hash to 
// a UUID string when the input hash is correct.
TEST(CheckStringConversion, correctHashToUUID)
{
	std::string input = "2c7f46d4f57cf9e66b03213358c7ddb5";

	std::string output = Utility::hashToUUIDString(input);
	ASSERT_EQ(output, "2c7f46d4-f57c-f9e6-6b03-213358c7ddb5");
}

// Checks if uuidStringToHash correctly converts a UUID 
// string to a hash when the input uuid string is correct.
TEST(CheckStringConversion, correctUUIDToHash)
{
	std::string input = "2c7f46d4-f57c-f9e6-6b03-213358c7ddb5";

	std::string output = Utility::uuidStringToHash(input);
	ASSERT_EQ(output, "2c7f46d4f57cf9e66b03213358c7ddb5");
}

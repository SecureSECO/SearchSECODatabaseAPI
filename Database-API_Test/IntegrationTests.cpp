/*This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University(Department of Informationand Computing Sciences)*/

#include "RequestHandler.h"
#include "DatabaseHandler.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

std::vector<std::string> splitStringOn(std::string str, char delimiter)
{
	std::stringstream strStream(str);
	std::string item;
	std::vector<std::string> substrings;
	while (std::getline(strStream, item, delimiter))
	{
		substrings.push_back(item);
	}
	return substrings;
}

// Tests check request functionality with a single known hash as input.
TEST(DatabaseIntegrationTest, CheckRequestSingleHash)
{
	// Set up:
	DatabaseHandler database;
	RequestHandler handler;
	handler.initialize(&database, "127.0.0.1", 9042);

	const std::string input1 = "2c7f46d4f57cf9e66b03213358c7ddb5";
	const std::string expectedOutput1 = "2c7f46d4f57cf9e66b03213358c7ddb5?1?5000000000000?M1?P1/M1.cpp?1?1?"
										"68bd2db6-fe91-47d2-a134-cf82b104f547\n";

	// Test:
	const std::string output1 = handler.handleRequest("chck", input1);
	ASSERT_EQ(output1, expectedOutput1);
}

// Tests check request functionality with unknown hash as input.
TEST(DatabaseIntegrationTest, CheckRequestUnknownHash)
{
	// Set up:
	DatabaseHandler database;
	RequestHandler handler;
	handler.initialize(&database, "127.0.0.1", 9042);

	const std::string input2 = "cb2b9a64f153e3947c5dafff0ce48949";
	const std::string expectedOutput2 = "No results found";

	// Test:
	const std::string output2 = handler.handleRequest("chck", input2);
	ASSERT_EQ(output2, expectedOutput2);
}

// Tests check request functionality with multiple hashes as input.
TEST(DatabaseIntegrationTest, CheckRequestMultipleHashes)
{
	// Set up:
	DatabaseHandler database;
	RequestHandler handler;
	handler.initialize(&database, "127.0.0.1", 9042);

	const std::string input3 = "8811e6bedb87e90cef39de1179f3bd2e\n137fed017b6159acc0af30d2c6b403a5";
	const std::string expectedOutput3_1 = "137fed017b6159acc0af30d2c6b403a5?3?5000000002000?M3?P3/M3.cpp?1?1?"
										  "b2217c08-06eb-4a57-b977-7c6d72299301";
	const std::string expectedOutput3_2 = "8811e6bedb87e90cef39de1179f3bd2e?4?5000000006000?M7?P4/M7.cpp?23?1?"
										  "f95ffc6c-aa97-40d6-b709-cb4823955213";
	const std::string expectedOutput3_3 = "8811e6bedb87e90cef39de1179f3bd2e?5?5000000009000?M10?P5/M10.cpp?61?1?"
										  "2a84cf5a-9554-4800-bb87-6dda6715fa12";
	std::vector<std::string> expectedOutputs3 = {expectedOutput3_1, expectedOutput3_2, expectedOutput3_3};

	// Test:
	std::string output3 = handler.handleRequest("chck", input3);
	std::vector<std::string> entries3 = splitStringOn(output3, '\n');

	// The number of entries should be equal to 3.
	ASSERT_EQ(entries3.size(), 3);

	// Check if the entries are inside expectedOutputs3.
	for (int i = 0; i < entries3.size(); i++)
	{
		std::vector<std::string>::iterator index3 =
			std::find(expectedOutputs3.begin(), expectedOutputs3.end(), entries3[i]);
		ASSERT_NE(index3, expectedOutputs3.end());
	}
}

// Tests check request functionality completely.
TEST(DatabaseIntegrationTest, CheckRequestComplete)
{
	// Set up:
	DatabaseHandler database;
	RequestHandler handler;
	handler.initialize(&database, "127.0.0.1", 9042);

	const std::string input4 =
		"137fed017b6159acc0af30d2c6b403a5\n7d5aad6f6fcc727d51b4859c17cbdb90\n23920776594c85fdc30cd96f928487f1";
	const std::string expectedOutput4_1_1 =
		"23920776594c85fdc30cd96f928487f1?3?5000000003000?M4?P3/M4.cpp?21?2?"
		"68bd2db6-fe91-47d2-a134-cf82b104f547?b2217c08-06eb-4a57-b977-7c6d72299301";
	const std::string expectedOutput4_1_2 =
		"23920776594c85fdc30cd96f928487f1?3?5000000003000?M4?P3/M4.cpp?21?2?"
		"b2217c08-06eb-4a57-b977-7c6d72299301?68bd2db6-fe91-47d2-a134-cf82b104f547";
	const std::string expectedOutput4_2_1 =
		"23920776594c85fdc30cd96f928487f1?5?5000000007000?M8?P5/M8.cpp?1?2?"
		"e39e0872-6856-4fa0-8d9a-278728362f43?f95ffc6c-aa97-40d6-b709-cb4823955213";
	const std::string expectedOutput4_2_2 =
		"23920776594c85fdc30cd96f928487f1?5?5000000007000?M8?P5/M8.cpp?1?2?"
		"f95ffc6c-aa97-40d6-b709-cb4823955213?e39e0872-6856-4fa0-8d9a-278728362f43";
	const std::string expectedOutput4_3 = "137fed017b6159acc0af30d2c6b403a5?3?5000000002000?M3?P3/M3.cpp?1?1?"
										  "b2217c08-06eb-4a57-b977-7c6d72299301";
	std::vector<std::string> expectedOutputs4 = {expectedOutput4_1_1, expectedOutput4_1_2, expectedOutput4_2_1,
												 expectedOutput4_1_2, expectedOutput4_3};

	// Test:
	std::string output4 = handler.handleRequest("chck", input4);
	std::vector<std::string> entries4 = splitStringOn(output4, '\n');

	// The number of entries should be equal to 3.
	ASSERT_EQ(entries4.size(), 3);

	// Check if the entries are inside expectedOutputs4.
	for (int i = 0; i < entries4.size(); i++)
	{
		std::vector<std::string>::iterator index4 =
			std::find(expectedOutputs4.begin(), expectedOutputs4.end(), entries4[i]);
		ASSERT_NE(index4, expectedOutputs4.end());
	}
}

// Tests upload request functionality with one method as input.
TEST(DatabaseIntegrationTest, UploadRequestOneMethod)
{
	// Set up:
	DatabaseHandler database;
	RequestHandler handler;
	handler.initialize(&database, "127.0.0.1", 9042);

	const std::string input5_1 = "6?5000000010000?L5?P6?www.github.com/p6?Author 8?author8@mail.com\n"
								 "a6aa62503e2ca3310e3a837502b80df5?M11?P6/M11.cpp?1?1?Author 8?author8@mail.com";
	const std::string input5_2 = "a6aa62503e2ca3310e3a837502b80df5";
	const std::string expectedOutput5_1 = "Your project is successfully added to the database.";
	const std::string unexpectedOutput5_2 = "No results found";

	// Test if output is correct:
	const std::string output5_1 = handler.handleRequest("upld", input5_1);
	ASSERT_EQ(output5_1, expectedOutput5_1);

	// Test if the method from the project is actually in the database:
	const std::string output5_2 = handler.handleRequest("chck", input5_2);
	ASSERT_NE(output5_2, unexpectedOutput5_2);
}

// Tests upload request functionality with multiple methods as input.
TEST(DatabaseIntegrationTest, UploadRequestMultipleMethods)
{
	// Set up:
	DatabaseHandler database;
	RequestHandler handler;
	handler.initialize(&database, "127.0.0.1", 9042);

	const std::string input6_1 = "7?5000000011000?L6?P7?www.github.com/p7?Author 9?author9@mail.com\n"
								 "88e1ad43ee7b716b7d19e5e65ee40da8?M12?P7/M12.cpp?1?2?"
								 "Author 7?author7@mail.com?Author 8?author8@mail.com\n"
								 "f3a258ba6cd26c1b7d553a493c614104?M13?P7/M13.cpp?41?1?"
								 "Author 8?author8@mail.com\n"
								 "59bf62494932580165af0451f76be3e9?M14?P7/M14.cpp?81?1?"
								 "Author 7?author7@mail.com";
	const std::string input6_2 =
		"88e1ad43ee7b716b7d19e5e65ee40da8\nf3a258ba6cd26c1b7d553a493c614104\n59bf62494932580165af0451f76be3e9";
	const std::string expectedOutput6 = "Your project is successfully added to the database.";

	// Test if output is correct:
	std::string output6_1 = handler.handleRequest("upld", input6_1);
	ASSERT_EQ(output6_1, expectedOutput6);

	// Test properties the data in the database should satisfy by doing a check request:
	std::string output6_2 = handler.handleRequest("chck", input6_2);

	// Test if the output has the right number of entries (which should be 3):
	ASSERT_EQ(std::count(output6_2.begin(), output6_2.end(), '\n'), 3);

	// Test if authorID generation works correctly by checking if there are exactly two authorIDs with frequency 2:
	std::vector<std::string> entries = splitStringOn(output6_2, '\n');
	std::vector<std::string> authorIDs = {};
	const int numberOfAuthorsIndex = 6;
	for (int i = 0; i < entries.size(); i++)
	{
		std::vector<std::string> entry = splitStringOn(entries[i], '?');
		int numberOfAuthors = std::stoi(entry[numberOfAuthorsIndex]);
		for (int j = 1; j <= numberOfAuthors; j++)
		{
			authorIDs.push_back(entry[numberOfAuthorsIndex + j]);
		}
	}
	ASSERT_EQ(authorIDs.size(), 4);

	std::sort(authorIDs.begin(), authorIDs.end());
	ASSERT_EQ(authorIDs[0], authorIDs[1]);
	ASSERT_NE(authorIDs[1], authorIDs[2]);
	ASSERT_EQ(authorIDs[2], authorIDs[3]);
}

// Tests checkupload request functionality with a known hash as input.
TEST(DatabaseIntegrationTest, CheckUploadRequestKnownHash)
{
	// Set up:
	DatabaseHandler database;
	RequestHandler handler;
	handler.initialize(&database, "127.0.0.1", 9042);

	const std::string input7 = "8?5000000012000?L7?P8?www.github.com/p8?Author 10?author10@mail.com\n"
							   "2c7f46d4f57cf9e66b03213358c7ddb5?M14?P8/M14.cpp?1?1?Author 10?author10@mail.com\n"
							   "d0b33728458eec4279cb91ee865414d5?M15?P8/M15.cpp?41?1?Author 10?author10@mail.com\n";
	const std::string expectedOutput7 = "2c7f46d4f57cf9e66b03213358c7ddb5?1?5000000000000?M1?P1/M1.cpp?1?1?"
										"68bd2db6-fe91-47d2-a134-cf82b104f547\n";

	// Test:
	const std::string output7 = handler.handleRequest("chup", input7);
	ASSERT_EQ(output7, expectedOutput7);
}
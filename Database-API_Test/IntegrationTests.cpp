/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "RequestHandler.h"
#include "DatabaseHandler.h"
#include "DatabaseConnection.h"
#include "HTTPStatus.h"
#include "RAFTConsensus.h"
#include "Utility.h"
#include "TestDefinitions.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>


// Tests check request functionality with a single known hash as input.
TEST(DatabaseIntegrationTest, CheckRequestSingleHash)
{
	// Set up.
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, TEST_IP, TEST_PORT);

	std::string input1 = "2c7f46d4f57cf9e66b03213358c7ddb5";

	std::vector<char> expectedChars = {};
	Utility::appendBy(expectedChars,
					  {input1, "1", "5000000000000", "9e350b124404f40a114509910619f641", "5000000000000",
					   "9e350b124404f40a114509910619f641", "M1", "P1/M1.cpp", "1", "1",
					   "1", "68bd2db6-fe91-47d2-a134-cf82b104f547"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string expectedOutput1(expectedChars.begin(), expectedChars.end());

	// Test:
	std::string output1 = handler.handleRequest("chck", input1, nullptr);
	ASSERT_EQ(output1, HTTPStatusCodes::success(expectedOutput1));
}

// Tests check request functionality with unknown hash as input.
TEST(DatabaseIntegrationTest, CheckRequestUnknownHash)
{
	// Set up.
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr, TEST_IP, TEST_PORT);

	std::string input2 = "cb2b9a64f153e3947c5dafff0ce48949";
	std::string expectedOutput2 = "No results found.";

	// Test:
	std::string output2 = handler.handleRequest("chck", input2, nullptr);
	ASSERT_EQ(output2, HTTPStatusCodes::success(expectedOutput2));
}

// Tests check request functionality with multiple hashes as input.
TEST(DatabaseIntegrationTest, CheckRequestMultipleHashes)
{
	// Set up.
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr, TEST_IP, TEST_PORT);

	std::vector<char> input3Chars = {};
	Utility::appendBy(input3Chars, {"8811e6bedb87e90cef39de1179f3bd2e", "137fed017b6159acc0af30d2c6b403a5"},
					  ENTRY_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string input3(input3Chars.begin(), input3Chars.end());

	std::vector<char> expectedChars = {};
	Utility::appendBy(expectedChars,
					  {"137fed017b6159acc0af30d2c6b403a5", "3", "5000000002000", "2d8b3b65caf0e9168a39be667be24861",
					   "5000000004000", "8a04d1e679548a35a6feec48321cac81", "M3", "P3/M3.cpp", "1", "1",
					   "1", "b2217c08-06eb-4a57-b977-7c6d72299301"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	expectedChars.pop_back();
	std::string expectedOutput3_1(expectedChars.begin(), expectedChars.end());

	expectedChars = {};
	Utility::appendBy(expectedChars,
					  {"8811e6bedb87e90cef39de1179f3bd2e", "4", "5000000006000", "6415e258c077e5cf3f98982d8050e941",
					   "5000000006000", "6415e258c077e5cf3f98982d8050e941", "M7", "P4/M7.cpp", "23", "1",
					   "1", "f95ffc6c-aa97-40d6-b709-cb4823955213"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	expectedChars.pop_back();
	std::string expectedOutput3_2(expectedChars.begin(), expectedChars.end());

	expectedChars = {};
	Utility::appendBy(expectedChars,
					  {"8811e6bedb87e90cef39de1179f3bd2e", "5", "5000000009000", "63308e3dbf0aba36ecaf66d5f51b6d2b",
					   "5000000009000", "63308e3dbf0aba36ecaf66d5f51b6d2b", "M10", "P5/M10.cpp", "61", "1",
					   "1", "2a84cf5a-9554-4800-bb87-6dda6715fa12"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	expectedChars.pop_back();
	std::string expectedOutput3_3(expectedChars.begin(), expectedChars.end());
	std::vector<std::string> expectedOutputs3 = {expectedOutput3_1, expectedOutput3_2, expectedOutput3_3};

	// Test:
	std::string output3 = handler.handleRequest("chck", input3, nullptr);
	std::vector<std::string> entries3 =
		Utility::splitStringOn(HTTPStatusCodes::getMessage(output3), ENTRY_DELIMITER_CHAR);

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
	// Set up.
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr, TEST_IP, TEST_PORT);

	std::vector<char> inputChars = {};
	Utility::appendBy(
		inputChars,
		{"137fed017b6159acc0af30d2c6b403a5", "7d5aad6f6fcc727d51b4859c17cbdb90", "23920776594c85fdc30cd96f928487f1"},
		ENTRY_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string input4(inputChars.begin(), inputChars.end());

	std::vector<char> expectedChars = {};
	Utility::appendBy(expectedChars,
					  {"23920776594c85fdc30cd96f928487f1", "3", "5000000003000", "e7b60fac745437880c1ccb8c4dd29f0f",
					   "5000000004000", "8a04d1e679548a35a6feec48321cac81", "M4", "P3/M4.cpp", "21", "1", "2",
					   "68bd2db6-fe91-47d2-a134-cf82b104f547", "b2217c08-06eb-4a57-b977-7c6d72299301"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	expectedChars.pop_back();
	std::string expectedOutput4_1_1(expectedChars.begin(), expectedChars.end());

	expectedChars = {};
	Utility::appendBy(expectedChars,
					  {"23920776594c85fdc30cd96f928487f1", "3", "5000000003000", "e7b60fac745437880c1ccb8c4dd29f0f",
					   "5000000004000", "8a04d1e679548a35a6feec48321cac81", "M4", "P3/M4.cpp", "21", "1", "2",
					   "b2217c08-06eb-4a57-b977-7c6d72299301", "68bd2db6-fe91-47d2-a134-cf82b104f547"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	expectedChars.pop_back();
	std::string expectedOutput4_1_2(expectedChars.begin(), expectedChars.end());

	expectedChars = {};
	Utility::appendBy(expectedChars,
					  {"23920776594c85fdc30cd96f928487f1", "5", "5000000007000", "e35706965bfee40184ae6b4f38c1c81d",
					   "5000000009000", "63308e3dbf0aba36ecaf66d5f51b6d2b", "M8", "P5/M8.cpp", "1", "1",
					   "2", "e39e0872-6856-4fa0-8d9a-278728362f43", "f95ffc6c-aa97-40d6-b709-cb4823955213"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	expectedChars.pop_back();
	std::string expectedOutput4_2_1(expectedChars.begin(), expectedChars.end());

	expectedChars = {};
	Utility::appendBy(expectedChars,
					  {"23920776594c85fdc30cd96f928487f1", "5", "5000000007000", "e35706965bfee40184ae6b4f38c1c81d",
					   "5000000009000", "63308e3dbf0aba36ecaf66d5f51b6d2b", "M8", "P5/M8.cpp", "1", "1",
					   "2", "f95ffc6c-aa97-40d6-b709-cb4823955213", "e39e0872-6856-4fa0-8d9a-278728362f43"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	expectedChars.pop_back();
	std::string expectedOutput4_2_2(expectedChars.begin(), expectedChars.end());

	expectedChars = {};
	Utility::appendBy(expectedChars,
					  {"137fed017b6159acc0af30d2c6b403a5", "3", "5000000002000", "2d8b3b65caf0e9168a39be667be24861",
					   "5000000004000", "8a04d1e679548a35a6feec48321cac81", "M3", "P3/M3.cpp", "1", "1",
					   "1", "b2217c08-06eb-4a57-b977-7c6d72299301"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	expectedChars.pop_back();
	std::string expectedOutput4_3(expectedChars.begin(), expectedChars.end());

	std::vector<std::string> expectedOutputs4 = {expectedOutput4_1_1, expectedOutput4_1_2, expectedOutput4_2_1,
												 expectedOutput4_1_2, expectedOutput4_3};

	// Test:
	std::string output4 = handler.handleRequest("chck", input4, nullptr);
	std::vector<std::string> entries4 = Utility::splitStringOn(HTTPStatusCodes::getMessage(output4), ENTRY_DELIMITER_CHAR);

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
	// Set up.
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr, TEST_IP, TEST_PORT);

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars,
					  {"6", "5000000010000", "e9c81d6c65846876fba53dede0cf6d72dbc04b57", "L5", "P6",
					   "www.github.com/p6", "Author 8", "author8@mail.com", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	inputChars.push_back(ENTRY_DELIMITER_CHAR);
	inputChars.push_back(ENTRY_DELIMITER_CHAR);
	Utility::appendBy(
		inputChars, {"a6aa62503e2ca3310e3a837502b80df5", "M11", "P6/M11.cpp", "1", "1", "Author 8", "author8@mail.com"},
		FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string input5_1(inputChars.begin(), inputChars.end());

	std::string input5_2 = "a6aa62503e2ca3310e3a837502b80df5";
	std::string expectedOutput5_1 = "Your project has been successfully added to the database.";
	std::string unexpectedOutput5_2 = "No results found.";

	// Test if output is correct:
	std::string output5_1 = handler.handleRequest("upld", input5_1, nullptr);
	ASSERT_EQ(output5_1, HTTPStatusCodes::success(expectedOutput5_1));

	// Test if the method from the project is actually in the database:
	std::string output5_2 = handler.handleRequest("chck", input5_2, nullptr);
	ASSERT_NE(output5_2, HTTPStatusCodes::success(unexpectedOutput5_2));
}

// Tests upload request functionality with multiple methods as input.
TEST(DatabaseIntegrationTest, UploadRequestMultipleMethods)
{
	// Set up.
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr, TEST_IP, TEST_PORT);

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars,
					  {"7", "5000000011000", "9f0d5304769444a78d1b31b39e56333b2116dd23", "L6", "P7",
					   "www.github.com/p7", "Author 9", "author9@mail.com", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	inputChars.push_back(ENTRY_DELIMITER_CHAR);
	inputChars.push_back(ENTRY_DELIMITER_CHAR);
	Utility::appendBy(inputChars,
					  {"88e1ad43ee7b716b7d19e5e65ee40da8", "M12", "P7/M12.cpp", "1", "2", "Author 7",
					   "author7@mail.com", "Author 8", "author8@mail.com"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(
		inputChars,
		{"f3a258ba6cd26c1b7d553a493c614104", "M13", "P7/M13.cpp", "41", "1", "Author 8", "author8@mail.com"},
		FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(
		inputChars,
		{"59bf62494932580165af0451f76be3e9", "M14", "P7/M14.cpp", "81", "1", "Author 7", "author7@mail.com"},
		FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string input6_1(inputChars.begin(), inputChars.end());

	inputChars = {};
	Utility::appendBy(
		inputChars,
		{"88e1ad43ee7b716b7d19e5e65ee40da8", "f3a258ba6cd26c1b7d553a493c614104", "59bf62494932580165af0451f76be3e9"},
		ENTRY_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string input6_2(inputChars.begin(), inputChars.end());
	std::string expectedOutput6 = "Your project has been successfully added to the database.";

	// Test if output is correct:
	std::string output6_1 = handler.handleRequest("upld", input6_1, nullptr);
	ASSERT_EQ(output6_1, HTTPStatusCodes::success(expectedOutput6));

	// Test properties the data in the database should satisfy by doing a check request:
	std::string output6_2 = handler.handleRequest("chck", input6_2, nullptr);

	// Test if the output has the right number of entries (which should be 3):
	std::string output6_2_message = HTTPStatusCodes::getMessage(output6_2);
	ASSERT_EQ(std::count(output6_2_message.begin(), output6_2_message.end(), ENTRY_DELIMITER_CHAR), 3);

	// Test if authorID generation works correctly by checking if there are exactly two authorIDs with frequency 2:
	std::vector<std::string> entries = Utility::splitStringOn(output6_2_message, ENTRY_DELIMITER_CHAR);
	std::vector<std::string> authorIDs = {};
	const int numberOfAuthorsIndex = 10;
	for (int i = 0; i < entries.size(); i++)
	{
		std::vector<std::string> entry = Utility::splitStringOn(entries[i], FIELD_DELIMITER_CHAR);
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
	// Set up.
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr, TEST_IP, TEST_PORT);

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars,
					  {"8", "5000000012000", "75936085c8bf9ea33be61ba51c96ee1abd5c38a3", "L7", "P8",
					   "www.github.com/p8", "Author 10", "author10@mail.com", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	inputChars.push_back(ENTRY_DELIMITER_CHAR);
	inputChars.push_back(ENTRY_DELIMITER_CHAR);
	Utility::appendBy(
		inputChars,
		{"2c7f46d4f57cf9e66b03213358c7ddb5", "M14", "P8/M14.cpp", "1", "1", "Author 10", "author10@mail.com"},
		FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(
		inputChars,
		{"d0b33728458eec4279cb91ee865414d5", "M15", "P8/M15.cpp", "41", "1", "Author 10", "author10@mail.com"},
		FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string input7(inputChars.begin(), inputChars.end());

	std::vector<char> expectedChars = {};
	Utility::appendBy(expectedChars,
					  {"2c7f46d4f57cf9e66b03213358c7ddb5", "1", "5000000000000", "9e350b124404f40a114509910619f641",
					   "5000000000000", "9e350b124404f40a114509910619f641", "M1", "P1/M1.cpp", "1", "1",
					   "1", "68bd2db6-fe91-47d2-a134-cf82b104f547"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string expectedOutput7(expectedChars.begin(), expectedChars.end());

	// Test:
	std::string output7 = handler.handleRequest("chup", input7, nullptr);
	ASSERT_EQ(output7, HTTPStatusCodes::success(expectedOutput7));
}

// Tests author by id request functionality with multiple known authors as input.
TEST(DatabaseIntegrationTest, GetAuthorRequestMultipleAuthor)
{
	// Set up.
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, TEST_IP, TEST_PORT);

	std::vector<char> requestChars = {};
	Utility::appendBy(requestChars, {"47919e8f-7103-48a3-9514-3f2d9d49ac61", "41ab7373-8f24-4a03-83dc-621036d99f34"},
					  ENTRY_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string request(requestChars.begin(), requestChars.end());

	std::vector<char> expectedChars = {};
	Utility::appendBy(expectedChars, {"Author1", "author1@mail.com", "47919e8f-7103-48a3-9514-3f2d9d49ac61"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(expectedChars, {"Author2", "author2@mail.com", "41ab7373-8f24-4a03-83dc-621036d99f34"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string expectedOutput1(expectedChars.begin(), expectedChars.end());

	expectedChars = {};
	Utility::appendBy(expectedChars, {"Author2", "author2@mail.com", "41ab7373-8f24-4a03-83dc-621036d99f34"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(expectedChars, {"Author1", "author1@mail.com", "47919e8f-7103-48a3-9514-3f2d9d49ac61"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string expectedOutput2(expectedChars.begin(), expectedChars.end());

	// Test:
	std::string output = handler.handleRequest("idau", request, nullptr);
	ASSERT_TRUE(output == HTTPStatusCodes::success(expectedOutput1) || output == HTTPStatusCodes::success(expectedOutput2));
}

// Tests author by id request functionality with an unknown author as input.
TEST(DatabaseIntegrationTest, GetAuthorRequestUnknownAuthor)
{
	// Set up.
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, TEST_IP, TEST_PORT);

	std::string request = "9e7eb5f5-2ff7-47ab-bfa0-4038e4afa280";
	std::string expectedOutput = "No results found.";

	// Test:
	std::string output = handler.handleRequest("idau", request, nullptr);
	ASSERT_EQ(output, HTTPStatusCodes::success(expectedOutput));
}

// Tests author by id request functionality with a known and unknown author as input.
TEST(DatabaseIntegrationTest, GetAuthorRequestSingleUnknownAuthor)
{
	// Set up.
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, TEST_IP, TEST_PORT);

	std::vector<char> requestChars = {};
	Utility::appendBy(requestChars, {"47919e8f-7103-48a3-9514-3f2d9d49ac61", "9e7eb5f5-2ff7-47ab-bfa0-4038e4afa280"},
					  ENTRY_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string request(requestChars.begin(), requestChars.end());

	std::vector<char> expectedChars = {};
	Utility::appendBy(expectedChars, {"Author1", "author1@mail.com", "47919e8f-7103-48a3-9514-3f2d9d49ac61"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string expectedOutput(expectedChars.begin(), expectedChars.end());

	// Test:
	std::string output = handler.handleRequest("idau", request, nullptr);
	ASSERT_EQ(output, HTTPStatusCodes::success(expectedOutput));
}

// Tests method by author request functionality with a single known author id as input.
TEST(DatabaseIntegrationTest, MethodByAuthorRequestSingleId)
{
	// Set up.
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, TEST_IP, TEST_PORT);

	std::string input = "41ab7373-8f24-4a03-83dc-621036d99f34";

	std::vector<char> expectedChars = {};
	Utility::appendBy(expectedChars,
					  {"41ab7373-8f24-4a03-83dc-621036d99f34", "137fed017b6159acc0af30d2c6b403a5", "69", "420"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string expectedOutput(expectedChars.begin(), expectedChars.end());

	// Test:
	std::string output = handler.handleRequest("aume", input, nullptr);
	ASSERT_EQ(output, HTTPStatusCodes::success(expectedOutput));
}

// Tests method by author request functionality with unknown author id as input.
TEST(DatabaseIntegrationTest, MethodByAuthorRequestUnknownId)
{
	// Set up.
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, TEST_IP, TEST_PORT);

	std::string input = "9e7eb5f5-2ff7-47ab-bfa0-4038e4afa280";
	std::string expectedOutput = "No results found.";

	// Test:
	std::string output = handler.handleRequest("aume", input, nullptr);
	ASSERT_EQ(output, HTTPStatusCodes::success(expectedOutput));
}

// Tests method by author request functionality with multiple author ids as input.
TEST(DatabaseIntegrationTest, MethodByAuthorRequestMultipleIds)
{
	// Set up.
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, TEST_IP, TEST_PORT);

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars, {"47919e8f-7103-48a3-9514-3f2d9d49ac61", "41ab7373-8f24-4a03-83dc-621036d99f34"},
					  ENTRY_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string input(inputChars.begin(), inputChars.end());

	std::vector<char> outputChars = {};
	Utility::appendBy(outputChars,
					  {"47919e8f-7103-48a3-9514-3f2d9d49ac61", "2c7f46d4f57cf9e66b03213358c7ddb5", "42", "69"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output1(outputChars.begin(), outputChars.end());

	outputChars = {};
	Utility::appendBy(outputChars,
					  {"47919e8f-7103-48a3-9514-3f2d9d49ac61", "06f73d7ab46184c55bf4742b9428a4c0", "42", "420"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output2(outputChars.begin(), outputChars.end());

	outputChars = {};
	Utility::appendBy(outputChars,
					  {"41ab7373-8f24-4a03-83dc-621036d99f34", "137fed017b6159acc0af30d2c6b403a5", "69", "420"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output3(outputChars.begin(), outputChars.end());

	// Test:
	std::string result = handler.handleRequest("aume", input, nullptr);

	EXPECT_EQ(result.size(), HTTPStatusCodes::success(output1).size() + output2.size() + output3.size());
	EXPECT_TRUE(result.find(output1) != std::string::npos);
	EXPECT_TRUE(result.find(output2) != std::string::npos);
	EXPECT_TRUE(result.find(output3) != std::string::npos);
}

// Tests method by author request functionality with a known and unknow author id as input.
TEST(DatabaseIntegrationTest, MethodByAuthorRequestMultipleIdsOneMatch)
{
	// Set up.
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, TEST_IP, TEST_PORT);

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars, {"41ab7373-8f24-4a03-83dc-621036d99f34", "9e7eb5f5-2ff7-47ab-bfa0-4038e4afa280"},
					  ENTRY_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string input(inputChars.begin(), inputChars.end());

	std::vector<char> outputChars = {};
	Utility::appendBy(outputChars,
					  {"41ab7373-8f24-4a03-83dc-621036d99f34", "137fed017b6159acc0af30d2c6b403a5", "69", "420"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string expectedOutput(outputChars.begin(), outputChars.end());

	// Test:
	std::string output = handler.handleRequest("aume", input, nullptr);

	ASSERT_EQ(output, HTTPStatusCodes::success(expectedOutput));
}


// Tests extract projects request functionaility with one existing project.
TEST(DatabaseIntegrationTest, ExtractProjectsRequestSingleExistingProject)
{
	// Set up.
	DatabaseHandler database;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	DatabaseConnection jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus, TEST_IP, TEST_PORT);

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars, {"1", "5000000000000"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string input9(inputChars.begin(), inputChars.end());

	std::vector<char> expectedChars = {};
	Utility::appendBy(expectedChars,
					  {"1", "5000000000000", "9e350b124404f40a114509910619f641", "L1", "P1", "www.github.com/p1",
					   "68bd2db6-fe91-47d2-a134-cf82b104f547", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string expected9(expectedChars.begin(), expectedChars.end());

	// Test:
	std::string output9 = handler.handleRequest("extp", input9, nullptr);
	ASSERT_EQ(output9, HTTPStatusCodes::success(expected9));
}

// Tests extract projects request with a non-existent project.
TEST(DatabaseIntegrationTest, ExtractProjectsRequestSingleNonExistingProject)
{
	// Set up.
	DatabaseHandler database;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	DatabaseConnection jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus, TEST_IP, TEST_PORT);

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars, {"1", "5000000001000"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string input10(inputChars.begin(), inputChars.end());

	std::string expected10 = "No results found.";

	// Test:
	std::string output10 = handler.handleRequest("extp", input10, nullptr);
	ASSERT_EQ(output10, HTTPStatusCodes::success(expected10));
}

// Tests extract projects request with multiple versions of the same project.
TEST(DatabaseIntegrationTest, ExtractProjectsRequestOneProjectMultipleVersions)
{
	// Set up.
	DatabaseHandler database;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	DatabaseConnection jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus, TEST_IP, TEST_PORT);

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars, {"101", "5000000008000"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(inputChars, {"101", "5000000009000"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string input11(inputChars.begin(), inputChars.end());

	std::vector<char> expectedChars = {};
	Utility::appendBy(expectedChars,
					  {"101", "5000000008000", "429ae78a6b15630c0ce5114d02b0c55f", "L5", "P101", "www.github.com/p101",
					   "e39e0872-6856-4fa0-8d9a-278728362f43", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	expectedChars.pop_back();
	std::string expectedOutput11_1(expectedChars.begin(), expectedChars.end());

	expectedChars = {};
	Utility::appendBy(expectedChars,
					  {"101", "5000000009000", "8be58ce8426f941e1f856bf5e4e14492", "L5", "P101", "www.github.com/p101",
					   "e39e0872-6856-4fa0-8d9a-278728362f43", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	expectedChars.pop_back();
	std::string expectedOutput11_2(expectedChars.begin(), expectedChars.end());

	std::vector<std::string> expectedOutputs11 = {expectedOutput11_1, expectedOutput11_2};

	// Test:
	std::string output11 = handler.handleRequest("extp", input11, nullptr);
	std::vector<std::string> entries11 = Utility::splitStringOn(HTTPStatusCodes::getMessage(output11), ENTRY_DELIMITER_CHAR);

	ASSERT_EQ(entries11.size(), 2);

	for (int i = 0; i < entries11.size(); i++)
	{
		std::vector<std::string>::iterator index11 =
			std::find(expectedOutputs11.begin(), expectedOutputs11.end(), entries11[i]);
		ASSERT_NE(index11, expectedOutputs11.end());
	}
}

// Tests extract projects request with different projects.
TEST(DatabaseIntegrationTest, ExtractProjectsRequestDifferentProjects)
{
	// Set up.
	DatabaseHandler database;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	DatabaseConnection jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus, TEST_IP, TEST_PORT);

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars, {"2", "5000000001000"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(inputChars, {"2", "5000000002000"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(inputChars, {"3", "5000000002000"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(inputChars, {"4", "5000000005000"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string input12(inputChars.begin(), inputChars.end());

	std::vector<char> expectedChars = {};
	Utility::appendBy(expectedChars,
					  {"2", "5000000001000", "9d075dfba5c2a903ff1f542ea729ae8b", "L2", "P2", "www.github.com/p2",
					   "68bd2db6-fe91-47d2-a134-cf82b104f547", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	expectedChars.pop_back();
	std::string expectedOutput12_1(expectedChars.begin(), expectedChars.end());

	expectedChars = {};
	Utility::appendBy(expectedChars,
					  {"3", "5000000002000", "2d8b3b65caf0e9168a39be667be24861", "L3", "P3", "www.github.com/p3",
					   "b2217c08-06eb-4a57-b977-7c6d72299301", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	expectedChars.pop_back();
	std::string expectedOutput12_2(expectedChars.begin(), expectedChars.end());

	expectedChars = {};
	Utility::appendBy(expectedChars,
					  {"4", "5000000005000", "70966cd9481793ab85a409374a66f36b", "L4", "P4", "www.github.com/p4",
					   "e39e0872-6856-4fa0-8d9a-278728362f43", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	expectedChars.pop_back();
	std::string expectedOutput12_3(expectedChars.begin(), expectedChars.end());

	std::vector<std::string> expectedOutputs12 = {expectedOutput12_1, expectedOutput12_2, expectedOutput12_3};

	// Test:
	std::string output12 = handler.handleRequest("extp", input12, nullptr);
	std::vector<std::string> entries12 = Utility::splitStringOn(HTTPStatusCodes::getMessage(output12), ENTRY_DELIMITER_CHAR);

	// Check if the number of found projects is equal to 3.
	ASSERT_EQ(entries12.size(), 3);

	// Ensure that the entries found correspond to these in the expectedOutput.
	for (int i = 0; i < entries12.size(); i++)
	{
		std::vector<std::string>::iterator index12 =
			std::find(expectedOutputs12.begin(), expectedOutputs12.end(), entries12[i]);
		ASSERT_NE(index12, expectedOutputs12.end());
	}
}


// Tests extract projects request functionaility with one existing project.
TEST(DatabaseIntegrationTest, PrevProjectsRequestSingleExistingProject)
{
	// Set up.
	DatabaseHandler database;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	DatabaseConnection jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus, TEST_IP, TEST_PORT);

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars, {"1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string input9(inputChars.begin(), inputChars.end());

	std::vector<char> expectedChars = {};
	Utility::appendBy(expectedChars,
					  {"1", "5000000000000", "9e350b124404f40a114509910619f641", "L1", "P1", "www.github.com/p1",
					   "68bd2db6-fe91-47d2-a134-cf82b104f547", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string expected9(expectedChars.begin(), expectedChars.end());

	// Test:
	std::string output9 = handler.handleRequest("gppr", input9, nullptr);
	ASSERT_EQ(output9, HTTPStatusCodes::success(expected9));
}

// Tests extract projects request with a non-existent project.
TEST(DatabaseIntegrationTest, PrevProjectsRequestSingleNonExistingProject)
{
	// Set up.
	DatabaseHandler database;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	DatabaseConnection jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus, TEST_IP, TEST_PORT);

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars, {"7864535641"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string input10(inputChars.begin(), inputChars.end());

	std::string expected10 = "No results found.";

	// Test:
	std::string output10 = handler.handleRequest("gppr", input10, nullptr);
	ASSERT_EQ(output10, HTTPStatusCodes::success(expected10));
}

// Tests extract projects request with multiple versions of the same project.
TEST(DatabaseIntegrationTest, PrevProjectsRequestOneProjectMultipleVersions)
{
	// Set up.
	DatabaseHandler database;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	DatabaseConnection jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus, TEST_IP, TEST_PORT);

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars, "101", ENTRY_DELIMITER_CHAR);
	std::string input11(inputChars.begin(), inputChars.end());

	std::vector<char> expectedChars = {};
	Utility::appendBy(expectedChars,
					  {"101", "5000000009000", "8be58ce8426f941e1f856bf5e4e14492", "L5", "P101", "www.github.com/p101",
					   "e39e0872-6856-4fa0-8d9a-278728362f43", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string expected(expectedChars.begin(), expectedChars.end());

	// Test:
	std::string output11 = handler.handleRequest("gppr", input11, nullptr);
	ASSERT_EQ(output11, HTTPStatusCodes::success(expected));
}

// Tests extract projects request with different projects.
TEST(DatabaseIntegrationTest, PrevProjectsRequestDifferentProjects)
{
	// Set up.
	DatabaseHandler database;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	DatabaseConnection jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus, TEST_IP, TEST_PORT);

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars, {"17938729387"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(inputChars, {"2"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(inputChars, {"3"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(inputChars, {"4"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string input12(inputChars.begin(), inputChars.end());

	std::vector<char> expectedChars = {};
	Utility::appendBy(
		expectedChars, {"2", "5000000001000", "9d075dfba5c2a903ff1f542ea729ae8b", "L2", "P2", "www.github.com/p2", "68bd2db6-fe91-47d2-a134-cf82b104f547", "1"},
		FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	expectedChars.pop_back();
	std::string expectedOutput12_1(expectedChars.begin(), expectedChars.end());

	expectedChars = {};
	Utility::appendBy(expectedChars,
					  {"3", "5000000004000", "8a04d1e679548a35a6feec48321cac81", "L3", "P3", "www.github.com/p3", "b2217c08-06eb-4a57-b977-7c6d72299301", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	expectedChars.pop_back();
	std::string expectedOutput12_2(expectedChars.begin(), expectedChars.end());

	expectedChars = {};
	Utility::appendBy(expectedChars,
					  {"4", "5000000006000", "6415e258c077e5cf3f98982d8050e941", "L4", "P4", "www.github.com/p4",
					   "e39e0872-6856-4fa0-8d9a-278728362f43", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	expectedChars.pop_back();
	std::string expectedOutput12_3(expectedChars.begin(), expectedChars.end());

	std::vector<std::string> expectedOutputs12 = {expectedOutput12_1, expectedOutput12_2, expectedOutput12_3};

	// Test:
	std::string output12 = handler.handleRequest("gppr", input12, nullptr);
	std::vector<std::string> entries12 = Utility::splitStringOn(HTTPStatusCodes::getMessage(output12), ENTRY_DELIMITER_CHAR);

	// Check if the number of found projects is equal to 3.
	ASSERT_EQ(entries12.size(), 3);

	// Ensure that the entries found correspond to these in the expectedOutput.
	for (int i = 0; i < entries12.size(); i++)
	{
		std::vector<std::string>::iterator index12 =
			std::find(expectedOutputs12.begin(), expectedOutputs12.end(), entries12[i]);
		ASSERT_NE(index12, expectedOutputs12.end());
	}
}

TEST(DatabaseIntegrationTest, UploadRequestUpdateProjectWithoutUnchangedFiles)
{
	// Set up.
	errno = 0;
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr, TEST_IP, TEST_PORT);

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars,
					  {"1", "5000000002000", "f3ae47acf1f7c2c23d208edb406f29928da29d30", "L1a", "P1a", "www.github.com/p1a", "Author 1", "author1@mail.com", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(inputChars, "5000000000000", ENTRY_DELIMITER_CHAR);
	Utility::appendBy(inputChars, "", ENTRY_DELIMITER_CHAR);
	Utility::appendBy(inputChars,
					  {"2807dac8e9f49ee4a24e14560c5d187a", "M1a", "P1a/M1a.cpp", "1", "1", "Author 1", "author1@mail.com"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string input(inputChars.begin(), inputChars.end());

	std::string input2 = "2807dac8e9f49ee4a24e14560c5d187a";
	std::string expectedOutput = "Your project has been successfully added to the database.";

	// Begin test:
	std::string output = handler.handleRequest("upld", input, nullptr);
	ASSERT_EQ(output, HTTPStatusCodes::success(expectedOutput));

	// Test if the updated project is uploaded correctly. (There should be a new project with projectID 1 and version
	// 5000000020000, with only the new method with the correct start- and endVersion).
	ProjectOut project = database.searchForProject(1, 5000000002000);
	ASSERT_EQ(project.hashes.size(), 1);

	// First check if the added method contains the correct start- and endVersion.
	std::string methodData = handler.handleRequest("chck", input2, nullptr);
	std::vector<std::string> methodDataEntries = Utility::splitStringOn(HTTPStatusCodes::getMessage(methodData), ENTRY_DELIMITER_CHAR);

	ASSERT_EQ(methodDataEntries.size(), 1);

	std::vector<std::string> methodDataFields = Utility::splitStringOn(methodDataEntries[0], FIELD_DELIMITER_CHAR);

	ASSERT_EQ(methodDataFields[2], "5000000002000");
	ASSERT_EQ(methodDataFields[4], "5000000002000");
}

TEST(DatabaseIntegrationTest, UploadRequestUpdateProjectWithUnchangedFile)
{
	// Set up.
	errno = 0;
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr, TEST_IP, TEST_PORT);

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars,
					  {"1", "5000000010000", "28d4ab0b9125c97da037d20982f9e90c1ac3a3f6", "L1a", "P1a", "www.github.com/p1a", "Author 1", "author1@mail.com", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(inputChars, "5000000000000", ENTRY_DELIMITER_CHAR);
	Utility::appendBy(inputChars, {"P1/M1.cpp"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(inputChars,
					  {"55040fde7161039911d5fe342c39853d", "M1a", "P1a/M1a.cpp", "1", "1", "Special Author 1",
					   "specialauthor1@mail.com"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string input(inputChars.begin(), inputChars.end());

	std::string input2 = "55040fde7161039911d5fe342c39853d";
	std::string expectedOutput = "Your project has been successfully added to the database.";

	// Begin test:
	std::string output = handler.handleRequest("upld", input, nullptr);
	ASSERT_EQ(output, HTTPStatusCodes::success(expectedOutput));

	// Test if the updated project is uploaded correctly. (There should be a new project with projectID 1 and version
	// 5000000020000, with both the unchanged method and the added method with the correct start- and endVersion).
	ProjectOut project = database.searchForProject(1, 5000000010000);
	ASSERT_EQ(project.hashes.size(), 2);

	// First check if the added method contains the correct start- and endVersion.
	std::string methodData = handler.handleRequest("chck", input2, nullptr);
	std::vector<std::string> methodDataEntries = Utility::splitStringOn(HTTPStatusCodes::getMessage(methodData), ENTRY_DELIMITER_CHAR);

	ASSERT_EQ(methodDataEntries.size(), 1);

	std::vector<std::string> methodDataFields = Utility::splitStringOn(methodDataEntries[0], FIELD_DELIMITER_CHAR);

	ASSERT_EQ(methodDataFields[2], "5000000010000");
	ASSERT_EQ(methodDataFields[4], "5000000010000");

	// Now also check if the unchanged method contained the correct start- and endVersion.
	methodData = handler.handleRequest("chck", "2c7f46d4f57cf9e66b03213358c7ddb5", nullptr);
	methodDataEntries = Utility::splitStringOn(HTTPStatusCodes::getMessage(methodData), ENTRY_DELIMITER_CHAR);

	for (int i = 0; i < methodDataEntries.size(); i++)
	{
		std::vector<std::string> methodDataFields = Utility::splitStringOn(methodDataEntries[i], FIELD_DELIMITER_CHAR);
		if (methodDataFields[0] == "1")
		{
			ASSERT_EQ(methodDataFields[2], "5000000000000");
			ASSERT_EQ(methodDataFields[4], "5000000010000");
		}
	}
}

TEST(DatabaseIntegrationTest, UploadRequestUpdateProjectWithBothChangedAndUnchangedPossiblyNonExistentFiles)
{
	// Set up.
	errno = 0;
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr, TEST_IP, TEST_PORT);

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars,
					  {"3", "5000000020000", "f51ef5f3bbg8580adc4c633e63c6cca9779c81a7", "L1a", "P1a", "www.github.com/p1a", "Author 1", "author1@mail.com", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(inputChars, "5000000004000", ENTRY_DELIMITER_CHAR);
	Utility::appendBy(inputChars, {"P3/M3.cpp", "P3/M4.cpp", "P1/UnknownFile.cpp"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(inputChars,
					  {"2808dac8e9f49ee4a24e14560c5d187a", "M1a", "P1a/M1a.cpp", "1", "1", "Author 1", "author1@mail.com"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string input(inputChars.begin(), inputChars.end());

	std::string input2 = "2808dac8e9f49ee4a24e14560c5d187a";
	std::string input3 = "137fed017b6159acc0af30d2c6b403a5";
	std::string input4 = "23920776594c85fdc30cd96f928487f1";
	std::string input5 = "959ee1ee12e6d6d87a4b6ee732aed9fc";
	std::string expectedOutput = "Your project has been successfully added to the database.";

	// Begin test:
	std::string output = handler.handleRequest("upld", input, nullptr);
	ASSERT_EQ(output, HTTPStatusCodes::success(expectedOutput));

	// Test if the updated project is uploaded correctly. (There should be a new project with projectID 1 and version
	// 5000000020000, with both the unchanged method and the added methods with the correct start- and endVersion).
	ProjectOut project = database.searchForProject(3, 5000000020000);
	ASSERT_EQ(project.hashes.size(), 3);

	// First check if the added method contains the correct start- and endVersion.
	std::string methodData = handler.handleRequest("chck", input2, nullptr);
	std::vector<std::string> methodDataEntries = Utility::splitStringOn(HTTPStatusCodes::getMessage(methodData), ENTRY_DELIMITER_CHAR);

	ASSERT_EQ(methodDataEntries.size(), 1);

	std::vector<std::string> methodDataFields = Utility::splitStringOn(methodDataEntries[0], FIELD_DELIMITER_CHAR);

	ASSERT_EQ(methodDataFields[2], "5000000020000");
	ASSERT_EQ(methodDataFields[4], "5000000020000");

	// Now also check if the unchanged methods contain the correct start- and endVersion.
	methodData = handler.handleRequest("chck", input3, nullptr);
	methodDataEntries = Utility::splitStringOn(HTTPStatusCodes::getMessage(methodData), ENTRY_DELIMITER_CHAR);

	for (int i = 0; i < methodDataEntries.size(); i++)
	{
		methodDataFields = Utility::splitStringOn(methodDataEntries[i], FIELD_DELIMITER_CHAR);
		if (methodDataFields[0] == "3")
		{
			ASSERT_EQ(methodDataFields[2], "5000000002000");
			ASSERT_EQ(methodDataFields[4], "5000000020000");
		}
	}

	methodData = handler.handleRequest("chck", input4, nullptr);
	methodDataEntries = Utility::splitStringOn(HTTPStatusCodes::getMessage(methodData), ENTRY_DELIMITER_CHAR);

	for (int i = 0; i < methodDataEntries.size(); i++)
	{
		methodDataFields = Utility::splitStringOn(methodDataEntries[i], FIELD_DELIMITER_CHAR);
		if (methodDataFields[0] == "3")
		{
			ASSERT_EQ(methodDataFields[2], "5000000003000");
			ASSERT_EQ(methodDataFields[4], "5000000020000");
		}
	}

	// Finally, check that a method inside a changed file contains the correct start- and endVersion.
	methodData = handler.handleRequest("chck", input5, nullptr);
	methodDataEntries = Utility::splitStringOn(HTTPStatusCodes::getMessage(methodData), ENTRY_DELIMITER_CHAR);

	for (int i = 0; i < methodDataEntries.size(); i++)
	{
		methodDataFields = Utility::splitStringOn(methodDataEntries[i], FIELD_DELIMITER_CHAR);
		if (methodDataFields[0] == "3")
		{
			ASSERT_EQ(methodDataFields[2], "5000000004000");
			ASSERT_EQ(methodDataFields[4], "5000000004000");
		}
	}
}
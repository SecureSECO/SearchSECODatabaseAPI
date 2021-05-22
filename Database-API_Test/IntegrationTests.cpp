/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "RequestHandler.h"
#include "DatabaseHandler.h"
#include "DatabaseConnection.h"
#include "RAFTConsensus.h"
#include "Utility.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

// Tests check request functionality with a single known hash as input.
TEST(DatabaseIntegrationTest, CheckRequestSingleHash)
{
	// Set up:
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, "127.0.0.1", 9042);

	const std::string input1 = "2c7f46d4f57cf9e66b03213358c7ddb5";
	const std::string expectedOutput1 = "2c7f46d4f57cf9e66b03213358c7ddb5?1?5000000000000?M1?P1/M1.cpp?1?1?"
										"68bd2db6-fe91-47d2-a134-cf82b104f547\n";

	// Test:
	const std::string output1 = handler.handleRequest("chck", input1, nullptr);
	ASSERT_EQ(output1, expectedOutput1);
}

// Tests check request functionality with unknown hash as input.
TEST(DatabaseIntegrationTest, CheckRequestUnknownHash)
{
	// Set up:
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr, "127.0.0.1", 9042);

	const std::string input2 = "cb2b9a64f153e3947c5dafff0ce48949";
	const std::string expectedOutput2 = "No results found.";

	// Test:
	const std::string output2 = handler.handleRequest("chck", input2, nullptr);
	ASSERT_EQ(output2, expectedOutput2);
}

// Tests check request functionality with multiple hashes as input.
TEST(DatabaseIntegrationTest, CheckRequestMultipleHashes)
{
	// Set up:
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr, "127.0.0.1", 9042);

	const std::string input3 = "8811e6bedb87e90cef39de1179f3bd2e\n137fed017b6159acc0af30d2c6b403a5";
	const std::string expectedOutput3_1 = "137fed017b6159acc0af30d2c6b403a5?3?5000000002000?M3?P3/M3.cpp?1?1?"
										  "b2217c08-06eb-4a57-b977-7c6d72299301";
	const std::string expectedOutput3_2 = "8811e6bedb87e90cef39de1179f3bd2e?4?5000000006000?M7?P4/M7.cpp?23?1?"
										  "f95ffc6c-aa97-40d6-b709-cb4823955213";
	const std::string expectedOutput3_3 = "8811e6bedb87e90cef39de1179f3bd2e?5?5000000009000?M10?P5/M10.cpp?61?1?"
										  "2a84cf5a-9554-4800-bb87-6dda6715fa12";
	std::vector<std::string> expectedOutputs3 = {expectedOutput3_1, expectedOutput3_2, expectedOutput3_3};

	// Test:
	std::string output3 = handler.handleRequest("chck", input3, nullptr);
	std::vector<std::string> entries3 = Utility::splitStringOn(output3, '\n');

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
	DatabaseConnection jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr, "127.0.0.1", 9042);

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
	std::string output4 = handler.handleRequest("chck", input4, nullptr);
	std::vector<std::string> entries4 = Utility::splitStringOn(output4, '\n');

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
	DatabaseConnection jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr, "127.0.0.1", 9042);

	const std::string input5_1 = "6?5000000010000?L5?P6?www.github.com/p6?Author 8?author8@mail.com\n"
								 "a6aa62503e2ca3310e3a837502b80df5?M11?P6/M11.cpp?1?1?Author 8?author8@mail.com";
	const std::string input5_2 = "a6aa62503e2ca3310e3a837502b80df5";
	const std::string expectedOutput5_1 = "Your project has been successfully added to the database.";
	const std::string unexpectedOutput5_2 = "No results found.";

	// Test if output is correct:
	const std::string output5_1 = handler.handleRequest("upld", input5_1, nullptr);
	ASSERT_EQ(output5_1, expectedOutput5_1);

	// Test if the method from the project is actually in the database:
	const std::string output5_2 = handler.handleRequest("chck", input5_2, nullptr);
	ASSERT_NE(output5_2, unexpectedOutput5_2);
}

// Tests upload request functionality with multiple methods as input.
TEST(DatabaseIntegrationTest, UploadRequestMultipleMethods)
{
	// Set up:
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr, "127.0.0.1", 9042);

	const std::string input6_1 = "7?5000000011000?L6?P7?www.github.com/p7?Author 9?author9@mail.com\n"
								 "88e1ad43ee7b716b7d19e5e65ee40da8?M12?P7/M12.cpp?1?2?"
								 "Author 7?author7@mail.com?Author 8?author8@mail.com\n"
								 "f3a258ba6cd26c1b7d553a493c614104?M13?P7/M13.cpp?41?1?"
								 "Author 8?author8@mail.com\n"
								 "59bf62494932580165af0451f76be3e9?M14?P7/M14.cpp?81?1?"
								 "Author 7?author7@mail.com";
	const std::string input6_2 =
		"88e1ad43ee7b716b7d19e5e65ee40da8\nf3a258ba6cd26c1b7d553a493c614104\n59bf62494932580165af0451f76be3e9";
	const std::string expectedOutput6 = "Your project has been successfully added to the database.";

	// Test if output is correct:
	std::string output6_1 = handler.handleRequest("upld", input6_1, nullptr);
	ASSERT_EQ(output6_1, expectedOutput6);

	// Test properties the data in the database should satisfy by doing a check request:
	std::string output6_2 = handler.handleRequest("chck", input6_2, nullptr);

	// Test if the output has the right number of entries (which should be 3):
	ASSERT_EQ(std::count(output6_2.begin(), output6_2.end(), '\n'), 3);

	// Test if authorID generation works correctly by checking if there are exactly two authorIDs with frequency 2:
	std::vector<std::string> entries = Utility::splitStringOn(output6_2, '\n');
	std::vector<std::string> authorIDs = {};
	const int numberOfAuthorsIndex = 6;
	for (int i = 0; i < entries.size(); i++)
	{
		std::vector<std::string> entry = Utility::splitStringOn(entries[i], '?');
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
	DatabaseConnection jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr, "127.0.0.1", 9042);

	const std::string input7 = "8?5000000012000?L7?P8?www.github.com/p8?Author 10?author10@mail.com\n"
							   "2c7f46d4f57cf9e66b03213358c7ddb5?M14?P8/M14.cpp?1?1?Author 10?author10@mail.com\n"
							   "d0b33728458eec4279cb91ee865414d5?M15?P8/M15.cpp?41?1?Author 10?author10@mail.com\n";
	const std::string expectedOutput7 = "2c7f46d4f57cf9e66b03213358c7ddb5?1?5000000000000?M1?P1/M1.cpp?1?1?"
										"68bd2db6-fe91-47d2-a134-cf82b104f547\n";

	// Test:
	const std::string output7 = handler.handleRequest("chup", input7, nullptr);
	ASSERT_EQ(output7, expectedOutput7);
}

// Tests id by author request functionality with multiple known authors as input.
TEST(DatabaseIntegrationTest, GetAuthorIdRequestMultipleAuthor)
{
	// Set up:
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, "127.0.0.1", 9042);

	std::string request = "Author1?author1@mail.com\nAuthor2?author2@mail.com\n";
	std::string expectedOutput1 = "Author1?author1@mail.com?47919e8f-7103-48a3-9514-3f2d9d49ac61\nAuthor2?author2@mail.com?"
						  "41ab7373-8f24-4a03-83dc-621036d99f34\n";
	std::string expectedOutput2 = "Author2?author2@mail.com?41ab7373-8f24-4a03-83dc-621036d99f34\nAuthor1?author1@mail.com?"
						  "47919e8f-7103-48a3-9514-3f2d9d49ac61\n";

	// Test:
	const std::string output = handler.handleRequest("auid", request, nullptr);
	ASSERT_TRUE(output == expectedOutput1 || output == expectedOutput2);
}

// Tests id by author request functionality with an unknown author as input.
TEST(DatabaseIntegrationTest, GetAuthorIdRequestUnknownAuthor)
{
	// Set up:
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, "127.0.0.1", 9042);

	std::string request = "UnknownAuthor?unknownauthor@mail.com\n";
	std::string expectedOutput = "No results found.";

	// Test:
	const std::string output = handler.handleRequest("auid", request, nullptr);
	ASSERT_EQ(output, expectedOutput);
}

// Tests id by author request functionality with a known and unknown author as input.
TEST(DatabaseIntegrationTest, GetAuthorIdRequestSingleUnknownAuthor)
{
	// Set up:
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, "127.0.0.1", 9042);

	std::string request = "Author1?author1@mail.com\nUnknownAuthor?unknownauthor2@mail.com\n";
	std::string expectedOutput = "Author1?author1@mail.com?47919e8f-7103-48a3-9514-3f2d9d49ac61\n";

	// Test:
	const std::string output = handler.handleRequest("auid", request, nullptr);
	ASSERT_EQ(output, expectedOutput);
}

// Tests author by id request functionality with multiple known authors as input.
TEST(DatabaseIntegrationTest, GetAuthorRequestMultipleAuthor)
{
	// Set up:
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, "127.0.0.1", 9042);

	std::string request = "47919e8f-7103-48a3-9514-3f2d9d49ac61\n41ab7373-8f24-4a03-83dc-621036d99f34\n";
	std::string expectedOutput1 = "Author1?author1@mail.com?47919e8f-7103-48a3-9514-3f2d9d49ac61\nAuthor2?author2@mail.com?"
						  "41ab7373-8f24-4a03-83dc-621036d99f34\n";
	std::string expectedOutput2 = "Author2?author2@mail.com?41ab7373-8f24-4a03-83dc-621036d99f34\nAuthor1?author1@mail.com?"
						  "47919e8f-7103-48a3-9514-3f2d9d49ac61\n";

	// Test:
	const std::string output = handler.handleRequest("idau", request, nullptr);
	ASSERT_TRUE(output == expectedOutput1 || output == expectedOutput2);
}

// Tests author by id request functionality with an unknown author as input.
TEST(DatabaseIntegrationTest, GetAuthorRequestUnknownAuthor)
{
	// Set up:
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, "127.0.0.1", 9042);

	std::string request = "9e7eb5f5-2ff7-47ab-bfa0-4038e4afa280\n";
	std::string expectedOutput = "No results found.";

	// Test:
	const std::string output = handler.handleRequest("idau", request, nullptr);
	ASSERT_EQ(output, expectedOutput);
}

// Tests author by id request functionality with a known and unknown author as input.
TEST(DatabaseIntegrationTest, GetAuthorRequestSingleUnknownAuthor)
{
	// Set up:
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, "127.0.0.1", 9042);

	std::string request = "47919e8f-7103-48a3-9514-3f2d9d49ac61\n9e7eb5f5-2ff7-47ab-bfa0-4038e4afa280\n";
	std::string expectedOutput = "Author1?author1@mail.com?47919e8f-7103-48a3-9514-3f2d9d49ac61\n";

	// Test:
	const std::string output = handler.handleRequest("idau", request, nullptr);
	ASSERT_EQ(output, expectedOutput);
}

// Tests method by author request functionality with a single known author id as input.
TEST(DatabaseIntegrationTest, MethodByAuthorRequestSingleId)
{
	// Set up:
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, "127.0.0.1", 9042);

	const std::string input = "41ab7373-8f24-4a03-83dc-621036d99f34\n";
	const std::string expectedOutput = "41ab7373-8f24-4a03-83dc-621036d99f34?137fed017b6159acc0af30d2c6b403a5?69?420\n";

	// Test:
	const std::string output = handler.handleRequest("aume", input, nullptr);
	ASSERT_EQ(output, expectedOutput);
}

// Tests method by author request functionality with unknown author id as input.
TEST(DatabaseIntegrationTest, MethodByAuthorRequestUnknownId)
{
	// Set up:
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, "127.0.0.1", 9042);

	const std::string input = "9e7eb5f5-2ff7-47ab-bfa0-4038e4afa280\n";
	const std::string expectedOutput = "No results found.";

	// Test:
	const std::string output = handler.handleRequest("aume", input, nullptr);
	ASSERT_EQ(output, expectedOutput);
}

// Tests method by author request functionality with multiple author ids as input.
TEST(DatabaseIntegrationTest, MethodByAuthorRequestMultipleIds)
{
	// Set up:
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, "127.0.0.1", 9042);

	const std::string input = "47919e8f-7103-48a3-9514-3f2d9d49ac61\n41ab7373-8f24-4a03-83dc-621036d99f34\n";
	std::string output1 = "47919e8f-7103-48a3-9514-3f2d9d49ac61?2c7f46d4f57cf9e66b03213358c7ddb5?42?69\n";
	std::string output2 = "47919e8f-7103-48a3-9514-3f2d9d49ac61?06f73d7ab46184c55bf4742b9428a4c0?42?420\n";
	std::string output3 = "41ab7373-8f24-4a03-83dc-621036d99f34?137fed017b6159acc0af30d2c6b403a5?69?420\n";

	// Test:
	std::string result = handler.handleRequest("aume", input, nullptr);

	EXPECT_EQ(result.size(), output1.size() + output2.size() + output3.size());
	EXPECT_TRUE(result.find(output1) != std::string::npos);
	EXPECT_TRUE(result.find(output2) != std::string::npos);
	EXPECT_TRUE(result.find(output3) != std::string::npos);
}

// Tests method by author request functionality with a known and unknow author id as input.
TEST(DatabaseIntegrationTest, MethodByAuthorRequestMultipleIdsOneMatch)
{
	// Set up:
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, "127.0.0.1", 9042);

	const std::string input = "41ab7373-8f24-4a03-83dc-621036d99f34\n9e7eb5f5-2ff7-47ab-bfa0-4038e4afa280\n";
	const std::string expectedOutput = "41ab7373-8f24-4a03-83dc-621036d99f34?137fed017b6159acc0af30d2c6b403a5?69?420\n";

	// Test:
	std::string output = handler.handleRequest("aume", input, nullptr);

	ASSERT_EQ(output, expectedOutput);
}


// Tests extract projects request functionaility with one existing project.
TEST(DatabaseIntegrationTest, ExtractProjectsRequestSingleExistingProject)
{
	// Set up:
	DatabaseHandler database;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
        DatabaseConnection jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus, "127.0.0.1", 9042);

	std::string input9 = "1?5000000000000";
	std::string expected9 = "1?5000000000000?L1?P1?www.github.com/p1?"
							"68bd2db6-fe91-47d2-a134-cf82b104f547\n";

	// Test:
	std::string output9 = handler.handleRequest("extp", input9, nullptr);
	ASSERT_EQ(output9, expected9);
}

// Tests extract projects request with a non-existent project.
TEST(DatabaseIntegrationTest, ExtractProjectsRequestSingleNonExistingProject)
{
	// Set up:
	DatabaseHandler database;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
        DatabaseConnection jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus, "127.0.0.1", 9042);

	std::string input10 = "1?5000000001000";
	std::string expected10 = "No results found.";

	// Test:
	std::string output10 = handler.handleRequest("extp", input10, nullptr);
	ASSERT_EQ(output10, expected10);
}

// Tests extract projects request with multiple versions of the same project.
TEST(DatabaseIntegrationTest, ExtractProjectsRequestOneProjectMultipleVersions)
{
	// Set up:
	DatabaseHandler database;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
        DatabaseConnection jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus, "127.0.0.1", 9042);

	std::string input11 = "101?5000000008000\n101?5000000009000";
	std::string expectedOutput11_1 =
		"101?5000000008000?L5?P101?www.github.com/p101?e39e0872-6856-4fa0-8d9a-278728362f43";
	std::string expectedOutput11_2 =
		"101?5000000009000?L5?P101?www.github.com/p101?e39e0872-6856-4fa0-8d9a-278728362f43";
	std::vector<std::string> expectedOutputs11 = {expectedOutput11_1, expectedOutput11_2};

	// Test:
	std::string output11 = handler.handleRequest("extp", input11, nullptr);
	std::vector<std::string> entries11 = Utility::splitStringOn(output11, '\n');

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
	// Set up:
	DatabaseHandler database;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
        DatabaseConnection jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus, "127.0.0.1", 9042);

	std::string input12 = "2?5000000001000\n2?5000000002000\n3?5000000002000\n4?5000000005000";
	std::string expectedOutput12_1 = "2?5000000001000?L2?P2?www.github.com/p2?68bd2db6-fe91-47d2-a134-cf82b104f547";
	std::string expectedOutput12_2 = "3?5000000002000?L3?P3?www.github.com/p3?b2217c08-06eb-4a57-b977-7c6d72299301";
	std::string expectedOutput12_3 = "4?5000000005000?L4?P4?www.github.com/p4?e39e0872-6856-4fa0-8d9a-278728362f43";
	std::vector<std::string> expectedOutputs12 = {expectedOutput12_1, expectedOutput12_2, expectedOutput12_3};

	// Test:
	std::string output12 = handler.handleRequest("extp", input12, nullptr);
	std::vector<std::string> entries12 = Utility::splitStringOn(output12, '\n');

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


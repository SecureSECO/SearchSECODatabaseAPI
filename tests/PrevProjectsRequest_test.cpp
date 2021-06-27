/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include "JDDatabaseMock.cpp"
#include "RaftConsensusMock.cpp"
#include "HTTPStatus.h"
#include "Utility.h"

#include <boost/shared_ptr.hpp>
#include <gtest/gtest.h>

// Checks if extract projects request works correctly when the request is empty.
TEST(ExtractPrevProjectsRequestTests, Empty)
{
	// Set up the test.
	errno - 0;

	RequestHandler handler;

	std::string input1 = "";
	std::string expected1 = "No results found.";

	// Check if the output is as expected.
	std::string output1 = handler.handleRequest("gppr", input1, nullptr);
	ASSERT_EQ(output1, HTTPStatusCodes::success(expected1));
}

// Checks if the extract projects request works correctly when searching for a single existing project.
TEST(ExtractPrevProjectsRequestTests, SingleExistingProject)
{
	// Set up the test.
	errno = 0;

	MockDatabase database;
	MockRaftConsensus raftConsensus;
	MockJDDatabase jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	ProjectID projectID2 = 1;

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars, {"1"}, FIELD_DELIMITER_CHAR, FIELD_DELIMITER_CHAR);
	std::string input2(inputChars.begin(), inputChars.end());

	ProjectOut project2 = {.projectID = 1,
						   .version = 4000000000000,
						   .versionHash = "9e350b124404f40a114509910619f641",
						   .license = "L1",
						   .name = "P1",
						   .url = "www.github.com/p1",
						   .ownerID = "68bd2db6-fe91-47d2-a134-cf82b104f547",
						   .hashes = {"2c7f46d4f57cf9e66b03213358c7ddb5"},
						   .parserVersion = 1};
	std::vector<char> expectedChars = {};
	Utility::appendBy(expectedChars,
					  {"1", "4000000000000", "9e350b124404f40a114509910619f641", "L1", "P1", "www.github.com/p1",
					   "68bd2db6-fe91-47d2-a134-cf82b104f547", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string expected2(expectedChars.begin(), expectedChars.end());

	EXPECT_CALL(database, prevProject(projectID2)).WillOnce(testing::Return(project2));

	// Check if the output is as expected.
	std::string output2 = handler.handleRequest("gppr", input2, nullptr);
	ASSERT_EQ(output2, HTTPStatusCodes::success(expected2));
}

// Checks if the extract projects request works correctly when searching for a non-existing project.
TEST(ExtractPrevProjectsRequestTests, SingleNonExistingProject)
{
	// Set up the test.
	errno = 0;

	MockDatabase database;
	MockRaftConsensus raftConsensus;
	MockJDDatabase jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	ProjectID projectID3 = 2497301;

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars, {"2497301"}, FIELD_DELIMITER_CHAR, FIELD_DELIMITER_CHAR);
	std::string input3(inputChars.begin(), inputChars.end());

	ProjectOut project;
	project.projectID = -1;
	std::string expected3 = "No results found.";

	EXPECT_CALL(database, prevProject(projectID3)).WillOnce(testing::SetErrnoAndReturn(ERANGE, project));

	// Check if the output is as expected.
	std::string output3 = handler.handleRequest("gppr", input3, nullptr);
	ASSERT_EQ(output3, HTTPStatusCodes::success(expected3));
}

// Checks if the extract projects request works correctly when searching for multiple projects.
TEST(ExtractPrevProjectsRequestTests, MultipleProjects)
{
	// Set up the test.
	errno = 0;

	MockDatabase database;
	MockRaftConsensus raftConsensus;
	MockJDDatabase jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	ProjectID projectID4_1 = 2;
	ProjectID projectID4_2 = 3;
	ProjectID projectID4_3 = 9274487;
	ProjectID projectID4_4 = 4;

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars, {"2", "3", "9274487", "4"}, ENTRY_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string input4(inputChars.begin(), inputChars.end());

	ProjectOut project4_1 = {.projectID = 2,
							 .version = 5000000001000,
							 .versionHash = "9d075dfba5c2a903ff1f542ea729ae8b",
							 .license = "L2",
							 .name = "P2",
							 .url = "www.github.com/p2",
							 .ownerID = "68bd2db6-fe91-47d2-a134-cf82b104f547",
							 .hashes = {"06f73d7ab46184c55bf4742b9428a4c0"},
							 .parserVersion = 1};
	ProjectOut project4_2 = {.projectID = 3,
							 .version = 5000000002000,
							 .versionHash = "2d8b3b65caf0e9168a39be667be24861",
							 .license = "L3",
							 .name = "P3",
							 .url = "www.github.com/p3",
							 .ownerID = "b2217c08-06eb-4a57-b977-7c6d72299301",
							 .hashes = {"137fed017b6159acc0af30d2c6b403a5", "23920776594c85fdc30cd96f928487f1",
										"959ee1ee12e6d6d87a4b6ee732aed9fc"},
							 .parserVersion = 1};
	ProjectOut project4_3;
	project4_3.projectID = -1;
	ProjectOut project4_4 = {.projectID = 4,
							 .version = 5000000005000,
							 .versionHash = "70966cd9481793ab85a409374a66f36b",
							 .license = "L4",
							 .name = "P4",
							 .url = "www.github.com/p4",
							 .ownerID = "e39e0872-6856-4fa0-8d9a-278728362f43",
							 .hashes = {"06f73d7ab46184c55bf4742b9428a4c0", "8811e6bedb87e90cef39de1179f3bd2e"},
							 .parserVersion = 1};
	std::vector<char> expectedChars1 = {};
	Utility::appendBy(expectedChars1,
					  {"2", "5000000001000", "9d075dfba5c2a903ff1f542ea729ae8b", "L2", "P2", "www.github.com/p2",
					   "68bd2db6-fe91-47d2-a134-cf82b104f547", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	expectedChars1.pop_back();
	std::string expected4_1(expectedChars1.begin(), expectedChars1.end());
	std::vector<char> expectedChars2 = {};
	Utility::appendBy(expectedChars2,
					  {"3", "5000000002000", "2d8b3b65caf0e9168a39be667be24861", "L3", "P3", "www.github.com/p3",
					   "b2217c08-06eb-4a57-b977-7c6d72299301", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	expectedChars2.pop_back();
	std::string expected4_2(expectedChars2.begin(), expectedChars2.end());
	std::vector<char> expectedChars3 = {};
	Utility::appendBy(expectedChars3,
					  {"4", "5000000005000", "70966cd9481793ab85a409374a66f36b", "L4", "P4", "www.github.com/p4",
					   "e39e0872-6856-4fa0-8d9a-278728362f43", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	expectedChars3.pop_back();
	std::string expected4_3(expectedChars3.begin(), expectedChars3.end());

	std::vector<std::string> expected4 = {expected4_1, expected4_2, expected4_3};
	EXPECT_CALL(database, prevProject(projectID4_1)).WillOnce(testing::Return(project4_1));
	EXPECT_CALL(database, prevProject(projectID4_2)).WillOnce(testing::Return(project4_2));
	EXPECT_CALL(database, prevProject(projectID4_3)).WillOnce(testing::SetErrnoAndReturn(ERANGE, project4_3));
	EXPECT_CALL(database, prevProject(projectID4_4)).WillOnce(testing::Return(project4_4));
	std::string output4 = handler.handleRequest("gppr", input4, nullptr);
	std::vector<std::string> entries4 =
		Utility::splitStringOn(HTTPStatusCodes::getMessage(output4), ENTRY_DELIMITER_CHAR);

	// Expect the status code to be succesfull.
	EXPECT_EQ(HTTPStatusCodes::getCode(output4), HTTPStatusCodes::getCode(HTTPStatusCodes::success("")));

	// Assert that the output contains 3 entries.
	ASSERT_EQ(entries4.size(), 3);

	// Make sure that the entries are as expected.
	for (int i = 0; i < entries4.size(); i++)
	{
		std::vector<std::string>::iterator index4 = std::find(expected4.begin(), expected4.end(), entries4[i]);
		ASSERT_NE(index4, expected4.end());
	}
}

// Tests if an input for the extract projects request with wrong argument types is correctly handled.
TEST(ExtractPrevProjectsRequestTests, WrongArgumentTypes)
{
	// Set up the test.
	errno = 0;

	RequestHandler handler;

	std::string input = "-";
	std::string expected6 = "The request failed. For each project, the projectID should be a long long int.";

	// Check if the output is as expected.
	std::string output6 = handler.handleRequest("gppr", input, nullptr);
	ASSERT_EQ(output6, HTTPStatusCodes::clientError(expected6));
}

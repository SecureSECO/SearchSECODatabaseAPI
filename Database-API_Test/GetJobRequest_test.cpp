/*This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)*/

#include "Definitions.h"
#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include "JDDatabaseMock.cpp"
#include "RaftConsensusMock.cpp"
#include "HTTPStatus.h"
#include "Utility.h"

#include <gtest/gtest.h>

// Test if a jobRequestHandler first returns crawl and then a job when the number of jobs it too low.
TEST(GetJobRequest, NotEnoughJobsTest)
{
	// Set up the test.
	errno = 0;
	
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;

	EXPECT_CALL(jddatabase, getNumberOfJobs()).WillOnce(testing::Return(3));
	handler.initialize(&database, &jddatabase, &raftConsensus);

	std::string requestType = "gtjb";
	std::string request = "";

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));

	std::string result = handler.handleRequest(requestType, request, nullptr);

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars, {"Crawl", "0"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	inputChars.pop_back();

	// Check if the first output is correct.
	std::string input(inputChars.begin(), inputChars.end());
	ASSERT_EQ(result, HTTPStatusCodes::success(input));

	EXPECT_CALL(jddatabase, getTopJob()).WillOnce(testing::Return("https://github.com/zavg/linux-0.01"));
	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	std::string result2 = handler.handleRequest(requestType, request, nullptr);

	std::vector<char> input2Chars = {};
	Utility::appendBy(input2Chars, {"Spider", "https://github.com/zavg/linux-0.01"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	input2Chars.pop_back();

	// Check if the second output is correct.
	std::string input2(input2Chars.begin(), input2Chars.end());
	ASSERT_EQ(result2, HTTPStatusCodes::success(input2));
}

// Test if job is returned when there are enough jobs in the database.
TEST(GetJobRequest, EnoughJobsTest)
{
	// Set up the test.
	errno = 0;

	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;

	EXPECT_CALL(jddatabase, getNumberOfJobs()).WillOnce(testing::Return(550));
	handler.initialize(&database, &jddatabase, &raftConsensus);

	std::string requestType = "gtjb";
	std::string request = "";

	EXPECT_CALL(jddatabase, getTopJob()).WillOnce(testing::Return("https://github.com/zavg/linux-0.01"));
	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	std::string result = handler.handleRequest(requestType, request, nullptr);

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars, {"Spider", "https://github.com/zavg/linux-0.01"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	inputChars.pop_back();

	// Check if the output is correct.
	std::string input(inputChars.begin(), inputChars.end());
	ASSERT_EQ(result, HTTPStatusCodes::success(input));
}

// Test if the right string is returned when there are no jobs in the database.
TEST(GetJobRequest, NoJobsTest)
{
	// Set up the test.
	errno = 0;

	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;

	EXPECT_CALL(jddatabase, getNumberOfJobs()).WillOnce(testing::Return(0));
	handler.initialize(&database, &jddatabase, &raftConsensus);

	std::string requestType = "gtjb";
	std::string request = "";

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));

	std::string result = handler.handleRequest(requestType, request, nullptr);

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars, {"Crawl", "0"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	inputChars.pop_back();

	// Check if the first output is correct.
	std::string input(inputChars.begin(), inputChars.end());
	ASSERT_EQ(result, HTTPStatusCodes::success(input));

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	std::string result2 = handler.handleRequest(requestType, request, nullptr);

	// Check if the second output is correct.
	ASSERT_EQ(result2, HTTPStatusCodes::success("NoJob"));
}

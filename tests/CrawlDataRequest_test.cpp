/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "Definitions.h"
#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include "JDDatabaseMock.cpp"
#include "RaftConsensusMock.cpp"
#include "HTTPStatus.h"
#include "Utility.h"
#include "StatisticsMock.cpp"

#include <gtest/gtest.h>

// Check if data is correctly handled when the crawlID is valid.
TEST(CrawlDataRequest, SingleJob)
{
	// Set up the test.
	errno = 0;

	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;
	MockStatistics stats;
	handler.initialize(&database, &jddatabase, &raftConsensus, &stats);

	std::string requestType = "upcd";
	std::vector<char> requestChars = {};
	Utility::appendBy(requestChars, {"100", "-1"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(requestChars, "", ENTRY_DELIMITER_CHAR);
	Utility::appendBy(requestChars, {"https://github.com/zavg/linux-0.01", "1"}, FIELD_DELIMITER_CHAR,
					  ENTRY_DELIMITER_CHAR);
	std::string request(requestChars.begin(), requestChars.end());

	EXPECT_CALL(raftConsensus, isLeader()).WillRepeatedly(testing::Return(true));

	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::success("Your job(s) has been succesfully added to the queue."));

	std::string requestType2 = "gtjb";
	std::string request2 = "";

	EXPECT_CALL(raftConsensus, isLeader()).WillRepeatedly(testing::Return(true));

	std::string result2 = handler.handleRequest(requestType2, "", request2, nullptr);

	std::vector<char> inputFunctionChars = {};
	Utility::appendBy(inputFunctionChars, {"Crawl", "100"}, FIELD_DELIMITER_CHAR, FIELD_DELIMITER_CHAR);
	inputFunctionChars.pop_back();

	// Check if the output is correct.
	std::string inputFunction(inputFunctionChars.begin(), inputFunctionChars.end());
	ASSERT_THAT(result2, testing::StartsWith(HTTPStatusCodes::success(inputFunction)));
}

// Check if an invalid crawlID is handled correctly.
TEST(CrawlDataRequest, InvalidID)
{
	// Set up the test.
	errno = 0;

	RequestHandler handler;
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr);

	std::string requestType = "upcd";
	std::vector<char> requestChars = {};
	Utility::appendBy(requestChars, {"aaa", "-1"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(requestChars, "", ENTRY_DELIMITER_CHAR);
	Utility::appendBy(requestChars, {"https://github.com/zavg/linux-0.01", "1"}, FIELD_DELIMITER_CHAR,
					  ENTRY_DELIMITER_CHAR);
	std::string request(requestChars.begin(), requestChars.end());

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));

	// Check if the output is correct.
	std::string result = handler.handleRequest(requestType, "", request, nullptr);
	ASSERT_EQ(result, HTTPStatusCodes::clientError("Error: invalid crawlID."));
}

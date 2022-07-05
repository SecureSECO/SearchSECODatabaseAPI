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

#include <gtest/gtest.h>

// Test if the ips are correctly returned.
TEST(GetIPsRequest, SingleIP)
{
	// Set up the test.
	errno = 0;

	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;

	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr);

	std::string requestType = "gtip";
	std::string request = "";

	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);
	std::string entryDelimiter(1, ENTRY_DELIMITER_CHAR);

	std::vector<std::string> ip = {"127.0.0.1" + fieldDelimiter + "-1"};

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(raftConsensus, getCurrentIPs()).WillOnce(testing::Return(ip));
	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars, {"Spider", "https://github.com/zavg/linux-0.01"}, FIELD_DELIMITER_CHAR,
					  ENTRY_DELIMITER_CHAR);
	inputChars.pop_back();

	// Check if the output is correct.
	std::string input(inputChars.begin(), inputChars.end());
	ASSERT_EQ(result, HTTPStatusCodes::success("127.0.0.1" + fieldDelimiter + "-1" + entryDelimiter));
}

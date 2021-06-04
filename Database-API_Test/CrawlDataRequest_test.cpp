/*This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)*/

#include "RequestHandler.h"
#include "HTTPStatus.h"
#include "DatabaseMock.cpp"
#include "JDDatabaseMock.cpp"
#include "RaftConsensusMock.cpp"
#include <gtest/gtest.h>

// Check if data is correctly handled when the crawlId is valid.
TEST(CrawlDataRequest, SingleJob)
{
	RequestHandler handler;
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	errno = 0;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	std::string requestType = "upcd";
	std::string request = "100\nhttps://github.com/zavg/linux-0.01?1";

	EXPECT_CALL(raftConsensus, isLeader()).WillRepeatedly(testing::Return(true));

	std::string result = handler.handleRequest(requestType, request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::success("Your job(s) has been succesfully added to the queue."));

	std::string requestType2 = "gtjb";
	std::string request2 = "";

	EXPECT_CALL(raftConsensus, isLeader()).WillRepeatedly(testing::Return(true));

	std::string result2 = handler.handleRequest(requestType2, request2, nullptr);

	ASSERT_EQ(result2, HTTPStatusCodes::success("Crawl?100"));
}

// Check if an invalid crawlId is handled correctly.
TEST(CrawlDataRequest, InvalidId)
{
	RequestHandler handler;
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	std::string requestType = "upcd";
	std::string request = "aaa\nhttps://github.com/zavg/linux-0.01?1";

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));

	std::string result = handler.handleRequest(requestType, request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::clientError("Error: invalid crawlId."));
}

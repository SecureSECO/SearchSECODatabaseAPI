/*This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)*/

#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include "JDDatabaseMock.cpp"
#include "RaftConsensusMock.cpp"
#include <gtest/gtest.h>

using namespace std;

//Check if data is correctly handled when the crawlId is valid.
TEST(CrawlDataRequest, SingleJob)
{
	RequestHandler handler;
        MockJDDatabase jddatabase;
        MockDatabase database;
        MockRaftConsensus raftConsensus;
        handler.initialize(&database, &jddatabase, &raftConsensus);

        string requestType = "upcd";
        string request = "100\nhttps://github.com/zavg/linux-0.01?1";

	EXPECT_CALL(jddatabase, updateCrawlId(100)).Times(1);
	EXPECT_CALL(raftConsensus, isLeader()).WillRepeatedly(testing::Return(true));

	string result = handler.handleRequest(requestType, request, nullptr);

	ASSERT_EQ(result, "Your job(s) has been succesfully added to the queue.");
}

//Check if an invalid crawlId is handled correctly.
TEST(CrawlDataRequest, InvalidId)
{
	RequestHandler handler;
        MockJDDatabase jddatabase;
        MockDatabase database;
        MockRaftConsensus raftConsensus;
        handler.initialize(&database, &jddatabase, &raftConsensus);

        string requestType = "upcd";
        string request = "aaa\nhttps://github.com/zavg/linux-0.01?1";

        EXPECT_CALL(jddatabase, updateCrawlId(100)).Times(0);
        EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));

        string result = handler.handleRequest(requestType, request, nullptr);

	ASSERT_EQ(result, "Error: invalid crawlId.");
}

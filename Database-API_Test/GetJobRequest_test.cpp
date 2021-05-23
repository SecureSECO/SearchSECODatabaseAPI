/*This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)*/

#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include "JDDatabaseMock.cpp"
#include "RaftConsensusMock.cpp"
#include <gtest/gtest.h>

using namespace std;



// Test if a jobRequestHandler first returns crawl and then a job when the number of jobs it too low.
TEST(GetJobRequest, NotEnoughJobsTest)
{
	RequestHandler handler;
        MockJDDatabase jddatabase;
        MockDatabase database;
        MockRaftConsensus raftConsensus;

	EXPECT_CALL(jddatabase, getNumberOfJobs()).WillOnce(testing::Return(3));
        handler.initialize(&database, &jddatabase, &raftConsensus);

	string requestType = "gtjb";
	string request = "";

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));

	string result = handler.handleRequest(requestType, request, nullptr);

	ASSERT_EQ(result, "Crawl?0");

	EXPECT_CALL(jddatabase, getTopJob()).WillOnce(testing::Return("https://github.com/zavg/linux-0.01"));
	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	string result2 = handler.handleRequest(requestType, request, nullptr);

	ASSERT_EQ(result2, "Spider?https://github.com/zavg/linux-0.01");
}

// Test if job is returned when there are enough jobs in the database.
TEST(GetJobRequest, EnoughJobsTest)
{
        RequestHandler handler;
        MockJDDatabase jddatabase;
        MockDatabase database;
        MockRaftConsensus raftConsensus;

        EXPECT_CALL(jddatabase, getNumberOfJobs()).WillOnce(testing::Return(550));
        handler.initialize(&database, &jddatabase, &raftConsensus);

        string requestType = "gtjb";
        string request = "";

        EXPECT_CALL(jddatabase, getTopJob()).WillOnce(testing::Return("https://github.com/zavg/linux-0.01"));
        EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
        string result = handler.handleRequest(requestType, request, nullptr);

        ASSERT_EQ(result, "Spider?https://github.com/zavg/linux-0.01");
}

// Test if the right string is returned when there are no jobs in the database.
TEST(GetJobRequest, NoJobsTest)
{
        RequestHandler handler;
        MockJDDatabase jddatabase;
        MockDatabase database;
        MockRaftConsensus raftConsensus;

        EXPECT_CALL(jddatabase, getNumberOfJobs()).WillOnce(testing::Return(0));
        handler.initialize(&database, &jddatabase, &raftConsensus);

        string requestType = "gtjb";
        string request = "";

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));

        string result = handler.handleRequest(requestType, request, nullptr);

        ASSERT_EQ(result, "Crawl?0");

        EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
        string result2 = handler.handleRequest(requestType, request, nullptr);

        ASSERT_EQ(result2, "NoJob");
}

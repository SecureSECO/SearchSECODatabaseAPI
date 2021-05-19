/*This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)*/

#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include "JDDatabaseMock.cpp"
#include "RaftConsensusMock.cpp"
#include <gtest/gtest.h>

using namespace std;

// Test for a single correct job.
TEST(UploadJobRequest, SingleJob)
{
	RequestHandler handler;
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	string requestType = "upjb";
	string request = "https://github.com/zavg/linux-0.01?1";

	EXPECT_CALL(jddatabase, uploadJob("https://github.com/zavg/linux-0.01", 1)).Times(1);
	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));

	string result = handler.handleRequest(requestType, request, nullptr);

	ASSERT_EQ(result, "Your job(s) has been succesfully added to the queue.");
}

// Test for multiple correct jobs.
TEST(UploadJobRequest, MultipleJobs)
{
        RequestHandler handler;
        MockJDDatabase jddatabase;
        MockDatabase database;
	MockRaftConsensus raftConsensus;
        handler.initialize(&database, &jddatabase, &raftConsensus);

        string requestType = "upjb";
        string request = "https://github.com/zavg/linux-0.01?1?https://github.com/nlohmann/json/issues/1573?2";

        EXPECT_CALL(jddatabase, uploadJob("https://github.com/zavg/linux-0.01", 1)).Times(1);
	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, uploadJob("https://github.com/nlohmann/json/issues/1573", 2)).Times(1);

        string result = handler.handleRequest(requestType, request, nullptr);
        ASSERT_EQ(result, "Your job(s) has been succesfully added to the queue.");
}

// Test for one job, but with an invalid priority.
TEST(UploadJobRequest, OneJobInvalidPriority)
{
        RequestHandler handler;
        MockJDDatabase jddatabase;
        MockDatabase database;
	MockRaftConsensus raftConsensus;
        handler.initialize(&database, &jddatabase, &raftConsensus);

	string requestType = "upjb";
        string request = "https://github.com/zavg/linux-0.01?aaaaa";

        EXPECT_CALL(jddatabase, uploadJob("https://github.com/zavg/linux-0.01", 0)).Times(0);
	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));

        string result = handler.handleRequest(requestType, request, nullptr);
        ASSERT_EQ(result, "A job has an invalid priority, no jobs have been added to the queue.");
}

//Test for multiple jobs, where one job has an invalid priority.
TEST(UploadJobRequest, MultipleJobsInvalidPriority)
{
        RequestHandler handler;
        MockJDDatabase jddatabase;
        MockDatabase database;
	MockRaftConsensus raftConsensus;
        handler.initialize(&database, &jddatabase, &raftConsensus);

        string requestType = "upjb";
        string request = "https://github.com/zavg/linux-0.01?1?https://github.com/nlohmann/json/issues/1573?aaaa";

        EXPECT_CALL(jddatabase, uploadJob("https://github.com/zavg/linux-0.01", 1)).Times(0);
	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
        EXPECT_CALL(jddatabase, uploadJob("https://github.com/nlohmann/json/issues/1573", 0)).Times(0);

        string result = handler.handleRequest(requestType, request, nullptr);
        ASSERT_EQ(result, "A job has an invalid priority, no jobs have been added to the queue.");
}

/*This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)*/

#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include "JDDatabaseMock.cpp"
#include "RaftConsensusMock.cpp"
#include <gtest/gtest.h>

using namespace std;

//Test if a job is succesfully returned.
TEST(GetJobRequest, BasicTest)
{
	RequestHandler handler;
        MockJDDatabase jddatabase;
        MockDatabase database;
        MockRaftConsensus raftConsensus;
        handler.initialize(&database, &jddatabase, &raftConsensus);

	string requestType = "gtjb";
	string request = "give job";

	EXPECT_CALL(jddatabase, getJob()).WillOnce(testing::Return("https://github.com/zavg/linux-0.01"));
	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));

	string result = handler.handleRequest(requestType, request, nullptr);

	ASSERT_EQ(result, "https://github.com/zavg/linux-0.01");
}

/*This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)*/

#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include "HTTPStatus.h"
#include "JDDatabaseMock.cpp"
#include "RaftConsensusMock.cpp"
#include <gtest/gtest.h>

// Test for a single correct job.
TEST(UploadJobRequest, SingleJob)
{
	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);

	RequestHandler handler;
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	errno = 0;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	std::string requestType = "upjb";
	std::string request = "https://github.com/zavg/linux-0.01" + fieldDelimiter + "1";

	EXPECT_CALL(jddatabase, uploadJob("https://github.com/zavg/linux-0.01", 1)).Times(1);
	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));

	std::string result = handler.handleRequest(requestType, request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::success("Your job(s) has been succesfully added to the queue."));
}

// Test for multiple correct jobs.
TEST(UploadJobRequest, MultipleJobs)
{
	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);
	std::string entryDelimiter(1, ENTRY_DELIMITER_CHAR);

	RequestHandler handler;
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	errno = 0;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	std::string requestType = "upjb";
	std::string request = "https://github.com/zavg/linux-0.01" + fieldDelimiter + "1" + entryDelimiter +
						  "https://github.com/nlohmann/json/issues/1573" + fieldDelimiter + "2";

	EXPECT_CALL(jddatabase, uploadJob("https://github.com/zavg/linux-0.01", 1)).Times(1);
	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, uploadJob("https://github.com/nlohmann/json/issues/1573", 2)).Times(1);

	std::string result = handler.handleRequest(requestType, request, nullptr);
	ASSERT_EQ(result, HTTPStatusCodes::success("Your job(s) has been succesfully added to the queue."));

}

// Test for one job, but with an invalid priority.
TEST(UploadJobRequest, OneJobInvalidPriority)
{
	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);

	RequestHandler handler;
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	std::string requestType = "upjb";
	std::string request = "https://github.com/zavg/linux-0.01" + fieldDelimiter + "aaaaa";

	EXPECT_CALL(jddatabase, uploadJob("https://github.com/zavg/linux-0.01", 0)).Times(0);
	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));

	std::string result = handler.handleRequest(requestType, request, nullptr);
	ASSERT_EQ(result, HTTPStatusCodes::clientError("A job has an invalid priority, no jobs have been added to the queue."));
}

//Test for multiple jobs, where one job has an invalid priority.
TEST(UploadJobRequest, MultipleJobsInvalidPriority)
{
	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);
	std::string entryDelimiter(1, ENTRY_DELIMITER_CHAR);

	RequestHandler handler;
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	errno = 0;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	std::string requestType = "upjb";
	std::string request = "https://github.com/zavg/linux-0.01" + fieldDelimiter + "1" + entryDelimiter + "https://github.com/nlohmann/json/issues/1573" + fieldDelimiter + "aaaa";

	EXPECT_CALL(jddatabase, uploadJob("https://github.com/zavg/linux-0.01", 1)).Times(0);
	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, uploadJob("https://github.com/nlohmann/json/issues/1573", 0)).Times(0);

	std::string result = handler.handleRequest(requestType, request, nullptr);
	ASSERT_EQ(result, HTTPStatusCodes::clientError("A job has an invalid priority, no jobs have been added to the queue."));
}

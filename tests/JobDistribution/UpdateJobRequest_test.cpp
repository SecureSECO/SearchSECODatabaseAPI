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

#include <gtest/gtest.h>

// Test for a single correct job.
TEST(UpdateJobRequest, CorrectJob)
{
	// Set up the test.
	errno = 0;

	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr);

	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);

	std::string requestType = "udjb";
	std::string request = "5d514d6e-2f23-fee7-b378-feda84ec123f" + fieldDelimiter + "420000";

	Job job;
	job.jobid = "5d514d6e-2f23-fee7-b378-feda84ec123f";
	job.time = 420000;
	job.timeout = 69;
	job.priority = 100;
	job.retries = 0;
	job.url = "https://github.com/zavg/linux-0.01";

	EXPECT_CALL(jddatabase, getCurrentJob("5d514d6e-2f23-fee7-b378-feda84ec123f")).WillOnce(testing::Return(job));
	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, addCurrentJob(job)).WillOnce(testing::Return(690000));

	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::success("690000"));
}

// Test for a single correct job.
TEST(UpdateJobRequest, UnknownJob)
{
	// Set up the test.
	errno = 0;

	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr);

	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);

	std::string requestType = "udjb";
	std::string request = "5d514d6e-2f23-fee7-b378-feda84ec123f" + fieldDelimiter + "420000";

	Job job;
	job.jobid = "";

	EXPECT_CALL(jddatabase, getCurrentJob("5d514d6e-2f23-fee7-b378-feda84ec123f")).WillOnce(testing::Return (job));
	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, addCurrentJob(job)).Times(0);

	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::clientError("Job not currently expected."));
}

// Test for a single correct job.
TEST(UpdateJobRequest, IncorrectJob)
{
	// Set up the test.
	errno = 0;

	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr);

	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);

	std::string requestType = "udjb";
	std::string request = "5d514d6e-2f23-fee7-b378-feda84ec123f" + fieldDelimiter + "420000";

	Job job;
	job.jobid = "5d514d6e-2f23-fee7-b378-feda84ec123f";
	job.time = 690000;
	job.timeout = 69;
	job.priority = 100;
	job.retries = 0;
	job.url = "https://github.com/zavg/linux-0.01";

	EXPECT_CALL(jddatabase, getCurrentJob("5d514d6e-2f23-fee7-b378-feda84ec123f")).WillOnce(testing::Return (job));
	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, addCurrentJob(job)).Times(0);

	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::clientError("Job not currently expected."));
}
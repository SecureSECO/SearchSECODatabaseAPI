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

TEST(FinishJobRequest, Success)
{
	// Set up the test.
	errno = 0;
	
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;
	
	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr);

	Job job;
	job.jobid = "58451e62-1794-4f03-8ae4-21fb42670f73";
	job.time = 0;
	job.timeout = 69;
	job.priority = 100;
	job.retries = 0;
	job.url = "https://github.com/zavg/linux-0.01";

	std::string requestType = "fnjb";
	std::string request = "58451e62-1794-4f03-8ae4-21fb42670f73?0?0?Success.\n";
	
	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, getCurrentJob(job.jobid)).WillOnce(testing::Return(Job));
	
	EXPECT_CALL(jddatabase, addFailedJob(FailedJob(job, 0, "")).Times(0);

	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::success("Job finished succesfully."));
}

TEST(FinishJobRequest, Failure)
{
	// Set up the test.
	errno = 0;
	
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;

	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr);

	Job job;
	job.jobid = "58451e62-1794-4f03-8ae4-21fb42670f73";
	job.time = 0;
	job.timeout = 69;
	job.priority = 100;
	job.retries = 0;
	job.url = "https://github.com/zavg/linux-0.01";

	std::string requestType = "fnjb";
	std::string request = "58451e62-1794-4f03-8ae4-21fb42670f73?0?10?Project already known.\n";

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, getCurrentJob(job.jobid)).WillOnce(testing::Return(Job));

	EXPECT_CALL(jddatabase, addFailedJob(FailedJob(job, 10, "Project already known.")).WillOnce(testing::Return(true));

	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::success("Job failed succesfully."));
}

TEST(FinishJobRequest, DeficientArguments)
{
	// Set up the test.
	errno = 0;
	
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;

	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr);

	Job job;
	job.jobid = "58451e62-1794-4f03-8ae4-21fb42670f73";
	job.time = 0;
	job.timeout = 69;
	job.priority = 100;
	job.retries = 0;
	job.url = "https://github.com/zavg/linux-0.01";

	std::string requestType = "fnjb";
	std::string request = "58451e62-1794-4f03-8ae4-21fb42670f73?0?0\n";

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, getCurrentJob(job.jobid)).Times(0);

	EXPECT_CALL(jddatabase, addFailedJob(FailedJob(job, 0, "")).Times(0);

	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::clientError("Incorrect amount of arguments."));
}

TEST(FinishJobRequest, IncorrectJobTime)
{
	// Set up the test.
	errno = 0;
	
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;

	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr);

	Job job;
	job.jobid = "58451e62-1794-4f03-8ae4-21fb42670f73";
	job.time = 0;
	job.timeout = 69;
	job.priority = 100;
	job.retries = 0;
	job.url = "https://github.com/zavg/linux-0.01";

	std::string requestType = "fnjb";
	std::string request = "58451e62-1794-4f03-8ae4-21fb42670f73?abc?0?Success.\n";

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, getCurrentJob(job.jobid)).Times(0);

	EXPECT_CALL(jddatabase, addFailedJob(FailedJob(job, 0, "")).Times(0);

	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::clientError("Incorrect job time."));
}

TEST(FinishJobRequest, IncorrectReasonID)
{
	// Set up the test.
	errno = 0;
	
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;

	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr);

	Job job;
	job.jobid = "58451e62-1794-4f03-8ae4-21fb42670f73";
	job.time = 0;
	job.timeout = 69;
	job.priority = 100;
	job.retries = 0;
	job.url = "https://github.com/zavg/linux-0.01";

	std::string requestType = "fnjb";
	std::string request = "58451e62-1794-4f03-8ae4-21fb42670f73?0?reason?Success.\n";

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, getCurrentJob(job.jobid)).Times(0);

	EXPECT_CALL(jddatabase, addFailedJob(FailedJob(job, 0, "")).Times(0);

	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::clientError("Incorrect reason id."));
}

TEST(FinishJobRequest, UnknownJob)
{
	// Set up the test.
	errno = 0;
	
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;

	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr);

	Job job;
	job.jobid = "";
	job.time = 0;
	job.timeout = 0;
	job.priority = 0;
	job.retries = 0;
	job.url = "";

	std::string requestType = "fnjb";
	std::string request = "58451e62-1794-4f03-8ae4-21fb42670f73?0?0?Success.\n";

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, getCurrentJob(job.jobid)).WillOnce(testing::Return(Job));
	
	EXPECT_CALL(jddatabase, addFailedJob(FailedJob(job, 0, "")).Times(0);

	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::clientError("Job not currently expected."));
}

TEST(FinishJobRequest, EarlyJob)
{
	// Set up the test.
	errno = 0;
	
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;

	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr);

	Job job;
	job.jobid = "58451e62-1794-4f03-8ae4-21fb42670f73";
	job.time = 0;
	job.timeout = 69;
	job.priority = 100;
	job.retries = 0;
	job.url = "https://github.com/zavg/linux-0.01";

	std::string requestType = "fnjb";
	std::string request = "58451e62-1794-4f03-8ae4-21fb42670f73?2?0?Success.\n";

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, getCurrentJob(job.jobid)).WillOnce(testing::Return(Job));
	
	EXPECT_CALL(jddatabase, addFailedJob(FailedJob(job, 0, "")).Times(0);

	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::clientError("Job not currently expected."));
}

TEST(FinishJobRequest, LateJob)
{
	// Set up the test.
	errno = 0;
	
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;

	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr);

	Job job;
	job.jobid = "58451e62-1794-4f03-8ae4-21fb42670f73";
	job.time = 2;
	job.timeout = 69;
	job.priority = 100;
	job.retries = 0;
	job.url = "https://github.com/zavg/linux-0.01";

	std::string requestType = "fnjb";
	std::string request = "58451e62-1794-4f03-8ae4-21fb42670f73?0?0?Success.\n";

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, getCurrentJob(job.jobid)).WillOnce(testing::Return(Job));
	
	EXPECT_CALL(jddatabase, addFailedJob(FailedJob(job, 0, "")).Times(0);

	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::clientError("Job not currently expected."));
}

TEST(FinishJobRequest, FailedJobFailure)
{
	// Set up the test.
	errno = 0;
	
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;

	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr);

	Job job;
	job.jobid = "58451e62-1794-4f03-8ae4-21fb42670f73";
	job.time = 2;
	job.timeout = 69;
	job.priority = 100;
	job.retries = 0;
	job.url = "https://github.com/zavg/linux-0.01";

	std::string requestType = "fnjb";
	std::string request = "58451e62-1794-4f03-8ae4-21fb42670f73?0?10?Project already known.\n";

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, getCurrentJob(job.jobid)).WillOnce(testing::Return(Job));
	
	EXPECT_CALL(jddatabase, addFailedJob(FailedJob(job, 10, "Project already known.")).WillOnce(testing::SetErrnoAndReturn(1, true));

	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::serverError("Job could not be added to failed jobs list."));
}
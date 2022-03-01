/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "Definitions.h"
#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include "StatisticsMock.cpp"
#include "JDDatabaseMock.cpp"
#include "RaftConsensusMock.cpp"
#include "HTTPStatus.h"
#include "Utility.h"

#include <gtest/gtest.h>

MATCHER_P(failedjobequal, job, "")
{
	return arg.jobid == job.jobid && arg.time == job.time && arg.timeout == job.timeout &&
		   arg.priority == job.priority && arg.url == job.url && arg.retries == job.retries &&
		   arg.reasonID == job.reasonID && arg.reasonData == job.reasonData;
}

MATCHER_P(jobequal, job, "")
{
	return arg.jobid == job.jobid && arg.timeout == job.timeout && arg.priority == job.priority && arg.url == job.url &&
		   arg.retries == job.retries;
}

ACTION_P(setErrno, error)
{
	errno = error;
}

TEST(FinishJobRequest, Success)
{
	// Set up the test.
	errno = 0;
	
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockStatistics stats;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;
	
	handler.initialize(&database, &jddatabase, &raftConsensus, &stats);

	Job job;
	job.jobid = "58451e62-1794-4f03-8ae4-21fb42670f73";
	job.time = 0;
	job.timeout = 69;
	job.priority = 100;
	job.retries = 0;
	job.url = "https://github.com/zavg/linux-0.01";

	std::string requestType = "fnjb";
	std::string request = "58451e62-1794-4f03-8ae4-21fb42670f73?0?0?Success.";
	
	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, getCurrentJobTime(job.jobid)).WillOnce(testing::Return(job.time));
	EXPECT_CALL(jddatabase, getCurrentJob(job.jobid)).WillOnce(testing::Return(job));
	
	EXPECT_CALL(jddatabase, addFailedJob(testing::_)).Times(0);
	EXPECT_CALL(jddatabase, uploadJob(testing::_, testing::_)).Times(0);

	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::success("Job finished succesfully."));
}

TEST(FinishJobRequest, Failure)
{
	// Set up the test.
	errno = 0;
	
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockStatistics stats;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;

	handler.initialize(&database, &jddatabase, &raftConsensus, &stats);

	Job job;
	job.jobid = "58451e62-1794-4f03-8ae4-21fb42670f73";
	job.time = 0;
	job.timeout = 69;
	job.priority = 100;
	job.retries = 0;
	job.url = "https://github.com/zavg/linux-0.01";

	std::string requestType = "fnjb";
	std::string request = "58451e62-1794-4f03-8ae4-21fb42670f73?0?11?Error downloading project.";

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, getCurrentJobTime(job.jobid)).WillOnce(testing::Return(job.time));
	EXPECT_CALL(jddatabase, getCurrentJob(job.jobid)).WillOnce(testing::Return(job));

	FailedJob failedJob = FailedJob(job, 11, "Error downloading project.");

	EXPECT_CALL(jddatabase, addFailedJob(failedjobequal(failedJob))).Times(1);
	job.retries++;
	EXPECT_CALL(jddatabase, uploadJob(jobequal(job), false)).Times(1);

	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::success("Job failed succesfully."));
}

TEST(FinishJobRequest, DeficientArguments)
{
	// Set up the test.
	errno = 0;
	
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockStatistics stats;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;

	handler.initialize(&database, &jddatabase, &raftConsensus, &stats);

	Job job;
	job.jobid = "58451e62-1794-4f03-8ae4-21fb42670f73";
	job.time = 0;
	job.timeout = 69;
	job.priority = 100;
	job.retries = 0;
	job.url = "https://github.com/zavg/linux-0.01";

	std::string requestType = "fnjb";
	std::string request = "58451e62-1794-4f03-8ae4-21fb42670f73?0?0";

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, getCurrentJobTime(testing::_)).Times(0);
	EXPECT_CALL(jddatabase, getCurrentJob(testing::_)).Times(0);

	EXPECT_CALL(jddatabase, addFailedJob(testing::_)).Times(0);
	EXPECT_CALL(jddatabase, uploadJob(testing::_, testing::_)).Times(0);

	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::clientError("Incorrect amount of arguments."));
}

TEST(FinishJobRequest, IncorrectJobTime)
{
	// Set up the test.
	errno = 0;
	
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockStatistics stats;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;

	handler.initialize(&database, &jddatabase, &raftConsensus, &stats);

	Job job;
	job.jobid = "58451e62-1794-4f03-8ae4-21fb42670f73";
	job.time = 0;
	job.timeout = 69;
	job.priority = 100;
	job.retries = 0;
	job.url = "https://github.com/zavg/linux-0.01";

	std::string requestType = "fnjb";
	std::string request = "58451e62-1794-4f03-8ae4-21fb42670f73?abc?0?Success.";

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, getCurrentJobTime(testing::_)).Times(0);
	EXPECT_CALL(jddatabase, getCurrentJob(testing::_)).Times(0);

	EXPECT_CALL(jddatabase, addFailedJob(testing::_)).Times(0);
	EXPECT_CALL(jddatabase, uploadJob(testing::_, testing::_)).Times(0);

	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::clientError("Incorrect job time."));
}

TEST(FinishJobRequest, IncorrectReasonID)
{
	// Set up the test.
	errno = 0;
	
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockStatistics stats;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;

	handler.initialize(&database, &jddatabase, &raftConsensus, &stats);

	Job job;
	job.jobid = "58451e62-1794-4f03-8ae4-21fb42670f73";
	job.time = 0;
	job.timeout = 69;
	job.priority = 100;
	job.retries = 0;
	job.url = "https://github.com/zavg/linux-0.01";

	std::string requestType = "fnjb";
	std::string request = "58451e62-1794-4f03-8ae4-21fb42670f73?0?reason?Success.";

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, getCurrentJobTime(testing::_)).Times(0);
	EXPECT_CALL(jddatabase, getCurrentJob(testing::_)).Times(0);

	EXPECT_CALL(jddatabase, addFailedJob(testing::_)).Times(0);
	EXPECT_CALL(jddatabase, uploadJob(testing::_, testing::_)).Times(0);

	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::clientError("Incorrect reason id."));
}

TEST(FinishJobRequest, UnknownJob)
{
	// Set up the test.
	errno = 0;
	
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockStatistics stats;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;

	handler.initialize(&database, &jddatabase, &raftConsensus, &stats);

	std::string requestType = "fnjb";
	std::string request = "58451e62-1794-4f03-8ae4-21fb42670f73?0?0?Success.";

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, getCurrentJobTime("58451e62-1794-4f03-8ae4-21fb42670f73")).WillOnce(testing::Return(-1));
	EXPECT_CALL(jddatabase, getCurrentJob(testing::_)).Times(0);
	
	EXPECT_CALL(jddatabase, addFailedJob(testing::_)).Times(0);
	EXPECT_CALL(jddatabase, uploadJob(testing::_, testing::_)).Times(0);

	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::clientError("Job not currently expected."));
}

TEST(FinishJobRequest, EarlyJob)
{
	// Set up the test.
	errno = 0;
	
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockStatistics stats;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;

	handler.initialize(&database, &jddatabase, &raftConsensus, &stats);

	std::string requestType = "fnjb";
	std::string request = "58451e62-1794-4f03-8ae4-21fb42670f73?2?0?Success.";

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, getCurrentJobTime("58451e62-1794-4f03-8ae4-21fb42670f73")).WillOnce(testing::Return(0));
	EXPECT_CALL(jddatabase, getCurrentJob(testing::_)).Times(0);
	
	EXPECT_CALL(jddatabase, addFailedJob(testing::_)).Times(0);
	EXPECT_CALL(jddatabase, uploadJob(testing::_, testing::_)).Times(0);

	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::clientError("Job not currently expected."));
}

TEST(FinishJobRequest, LateJob)
{
	// Set up the test.
	errno = 0;
	
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockStatistics stats;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;

	handler.initialize(&database, &jddatabase, &raftConsensus, &stats);

	std::string requestType = "fnjb";
	std::string request = "58451e62-1794-4f03-8ae4-21fb42670f73?0?0?Success.";

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, getCurrentJobTime("58451e62-1794-4f03-8ae4-21fb42670f73")).WillOnce(testing::Return(2));
	EXPECT_CALL(jddatabase, getCurrentJob(testing::_)).Times(0);
	
	EXPECT_CALL(jddatabase, addFailedJob(testing::_)).Times(0);
	EXPECT_CALL(jddatabase, uploadJob(testing::_, testing::_)).Times(0);

	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::clientError("Job not currently expected."));
}

TEST(FinishJobRequest, FailedJobFailure)
{
	// Set up the test.
	errno = 0;
	
	MockJDDatabase jddatabase;
	MockDatabase database;
	MockStatistics stats;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;

	handler.initialize(&database, &jddatabase, &raftConsensus, &stats);

	Job job;
	job.jobid = "58451e62-1794-4f03-8ae4-21fb42670f73";
	job.time = 0;
	job.timeout = 69;
	job.priority = 100;
	job.retries = 0;
	job.url = "https://github.com/zavg/linux-0.01";

	std::string requestType = "fnjb";
	std::string request = "58451e62-1794-4f03-8ae4-21fb42670f73?0?10?Project already known.";

	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, getCurrentJobTime(job.jobid)).WillOnce(testing::Return(job.time));
	EXPECT_CALL(jddatabase, getCurrentJob(job.jobid)).WillOnce(testing::Return(job));
	
	EXPECT_CALL(jddatabase, addFailedJob(failedjobequal(FailedJob(job, 10, "Project already known.")))).Times(MAX_RETRIES + 1).WillRepeatedly(setErrno(1));
	EXPECT_CALL(jddatabase, uploadJob(testing::_, testing::_)).Times(0);

	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::serverError("Job could not be added to failed jobs list."));
}
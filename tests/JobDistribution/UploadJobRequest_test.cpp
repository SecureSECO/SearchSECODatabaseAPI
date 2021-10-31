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

MATCHER_P(jobequal, job, "")
{
	return arg.jobid == job.jobid && arg.timeout == job.timeout && arg.priority == job.priority && arg.url == job.url &&
		   arg.retries == job.retries;
}

// Test for a single correct job.
TEST(UploadJobRequest, SingleJob)
{
	// Set up the test.
	errno = 0;

	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr);

	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);

	std::string requestType = "upjb";
	std::string request = "https://github.com/zavg/linux-0.01" + fieldDelimiter + "1" + fieldDelimiter + "69";

	Job job("", 69, 1, "https://github.com/zavg/linux-0.01", 0);

	EXPECT_CALL(jddatabase, uploadJob(jobequal(job), true)).Times(1);
	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));

	std::string result = handler.handleRequest(requestType, "", request, nullptr);

	ASSERT_EQ(result, HTTPStatusCodes::success("Your job(s) has been succesfully added to the queue."));
}

// Test for multiple correct jobs.
TEST(UploadJobRequest, MultipleJobs)
{
	// Set up the test.
	errno = 0;

	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr);

	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);
	std::string entryDelimiter(1, ENTRY_DELIMITER_CHAR);

	std::string requestType = "upjb";
	std::string request = "https://github.com/zavg/linux-0.01" + fieldDelimiter + "1" + fieldDelimiter + "69" + entryDelimiter +
						  "https://github.com/nlohmann/json/issues/1573" + fieldDelimiter + "2" + fieldDelimiter + "42";

	Job job1("", 69, 1, "https://github.com/zavg/linux-0.01", 0);
	Job job2("", 42, 2, "https://github.com/nlohmann/json/issues/1573", 0);

	EXPECT_CALL(jddatabase, uploadJob(jobequal(job1), true)).Times(1);
	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));
	EXPECT_CALL(jddatabase, uploadJob(jobequal(job2), true)).Times(1);

	std::string result = handler.handleRequest(requestType, "", request, nullptr);
	ASSERT_EQ(result, HTTPStatusCodes::success("Your job(s) has been succesfully added to the queue."));

}

// Test for one job, but with an invalid priority.
TEST(UploadJobRequest, OneJobInvalidPriority)
{
	// Set up the test.
	errno = 0;

	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr);

	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);

	std::string requestType = "upjb";
	std::string request = "https://github.com/zavg/linux-0.01" + fieldDelimiter + "aaaaa" + fieldDelimiter + "69";

	EXPECT_CALL(jddatabase, uploadJob(testing::_, testing::_)).Times(0);
	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));

	std::string result = handler.handleRequest(requestType, "", request, nullptr);
	ASSERT_EQ(result,
			  HTTPStatusCodes::clientError("A job has an invalid priority, no jobs have been added to the queue."));
}

//Test for multiple jobs, where one job has an invalid priority.
TEST(UploadJobRequest, MultipleJobsInvalidPriority)
{
	// Set up the test.
	errno = 0;

	MockJDDatabase jddatabase;
	MockDatabase database;
	MockRaftConsensus raftConsensus;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr);

	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);
	std::string entryDelimiter(1, ENTRY_DELIMITER_CHAR);

	std::string requestType = "upjb";
	std::string request = "https://github.com/zavg/linux-0.01" + fieldDelimiter + "1" + fieldDelimiter + "69" + entryDelimiter +
						  "https://github.com/nlohmann/json/issues/1573" + fieldDelimiter + "aaaa" + fieldDelimiter + "42";

	EXPECT_CALL(jddatabase, uploadJob(testing::_, testing::_)).Times(0);
	EXPECT_CALL(raftConsensus, isLeader()).WillOnce(testing::Return(true));

	std::string result = handler.handleRequest(requestType, "", request, nullptr);
	ASSERT_EQ(result,
			  HTTPStatusCodes::clientError("A job has an invalid priority, no jobs have been added to the queue."));
}

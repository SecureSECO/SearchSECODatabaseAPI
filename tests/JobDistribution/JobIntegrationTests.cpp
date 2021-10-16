/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "Definitions.h"
#include "TestDefinitions.h"
#include "RequestHandler.h"
#include "DatabaseConnection.h"
#include "DatabaseHandler.h"
#include "RAFTConsensus.h"
#include "HTTPStatus.h"
#include "Utility.h"
#include "StatisticsMock.cpp"

#include <algorithm>
#include <chrono>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <vector>


// Test if first crawl is returned and then the correct job.
TEST(JobDatabaseIntegrationTest, GetJobRequest)
{
	// Set up the test.
	errno = 0;

	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RAFTConsensus raftConsensus(nullptr);
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr, TEST_IP, TEST_PORT);

	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);

	std::string input = "";
	std::string expectedOutput = "Crawl" + fieldDelimiter + "0" + fieldDelimiter;

	// Check if the first output is as expected.
	std::string output = handler.handleRequest("gtjb", "", input, nullptr);
	EXPECT_THAT(output, testing::StartsWith(HTTPStatusCodes::success(expectedOutput)));

	std::string expectedOutput2 = "Spider" + fieldDelimiter + "2f78a799-0e18-4eb5-bd33-83718a3257d9" + fieldDelimiter + "https://github.com/caged/microsis";

	// Check if the second output is as expected.
	std::string output2 = handler.handleRequest("gtjb", "", input, nullptr);
	ASSERT_THAT(output2, testing::StartsWith(HTTPStatusCodes::success(expectedOutput2)));
}

// Test if job is succesfully uploaded and the numberOfJobs variable is increased
// and the job is inserted correctly in the database.
TEST(JobDatabaseIntegrationTest, UploadJobRequest)
{
	// Set up the test.
	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);

	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus(nullptr);
	MockStatistics stats;
	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr, TEST_IP, TEST_PORT);

	raftConsensus.start(&handler, {}, true);

	std::string input = "https://github.com/mcostalba/Stockfish" + fieldDelimiter + "10" + fieldDelimiter + "99999";
	std::string output = handler.handleRequest("upjb", "", input, nullptr);
	JobRequestHandler *jhandler = new JobRequestHandler(&raftConsensus, &handler, &jddatabase, &stats, TEST_IP, TEST_PORT);
	int jobs = jddatabase.getNumberOfJobs();
	EXPECT_EQ(jobs, 3);

	std::string input2 = "";
	std::string expectedOutput2 = "Crawl" + fieldDelimiter + "0";

	// Check if the first output is as expected.
	std::string output2 = handler.handleRequest("gtjb", "", input2, nullptr);
	EXPECT_THAT(output2, testing::StartsWith(HTTPStatusCodes::success(expectedOutput2)));

	// Check if the second output is as expected.
	std::string output3 = handler.handleRequest("gtjb", "", input2, nullptr);
	EXPECT_THAT(output3, testing::StartsWith(HTTPStatusCodes::success("Spider" + fieldDelimiter)));
	ASSERT_THAT(output3, testing::HasSubstr("https://github.com/mcostalba/Stockfish"));
}

// Test if multiple jobs get succesfully uploaded.
TEST(JobDatabaseIntegrationTest, UploadMulitpleJobs)
{
	// Set up the test.
	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);
	std::string entryDelimiter(1, ENTRY_DELIMITER_CHAR);

	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus(nullptr);
	MockStatistics stats;
	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr, TEST_IP, TEST_PORT);

	raftConsensus.start(&handler, {}, true);

	std::string input = "https://github.com/HackerPoet/Chaos-Equations" + fieldDelimiter + "42" + fieldDelimiter + "99998" + entryDelimiter +
						"https://github.com/Yiziwinnie/Home-Depot" + fieldDelimiter + "69" + fieldDelimiter + "99997";
	std::string output = handler.handleRequest("upjb", "", input, nullptr);
	JobRequestHandler *jhandler = new JobRequestHandler(&raftConsensus, &handler, &jddatabase, &stats, TEST_IP, TEST_PORT);
	int jobs = jddatabase.getNumberOfJobs();
	ASSERT_EQ(jobs, 4);
}

// Test if crawl data is handled succesfully.
TEST(JobDatabaseIntegrationTest, CrawlDataRequest)
{
	// Set up the test.
	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);
	std::string entryDelimiter(1, ENTRY_DELIMITER_CHAR);

	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus(nullptr);
	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr, TEST_IP, TEST_PORT);

	raftConsensus.start(&handler, {}, true);

	std::string input =
		"100" + fieldDelimiter + "-1" + entryDelimiter + "" + entryDelimiter + "https://github.com/Yiziwinnie/Bike-Sharing-in-Boston" + fieldDelimiter + "420" + fieldDelimiter + "99996";
	std::string output = handler.handleRequest("upcd", "", input, nullptr);

	std::string input2 = "";
	std::string output2 = handler.handleRequest("gtjb", "", input2, nullptr);
	std::string expectedOutput2 = "Crawl" + fieldDelimiter + "100" + fieldDelimiter;

	// Check if the output is as expected.
	ASSERT_THAT(output2, testing::StartsWith(HTTPStatusCodes::success(expectedOutput2)));
}

// Test if updating a job is handled succesfully.
TEST(JobDatabaseIntegrationTest, UpdateJobRequest)
{
	// Set up the test.
	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);

	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus(nullptr);
	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr, TEST_IP, TEST_PORT);

	raftConsensus.start(&handler, {}, true);

	// First take the crawl job.
	std::string input = "";
	std::string output = handler.handleRequest("gtjb", "", input, nullptr);

	// Take an actual job.
	std::string output2 = handler.handleRequest("gtjb", "", input, nullptr);
	ASSERT_THAT(output2, testing::StartsWith(HTTPStatusCodes::success("Spider")));

	std::vector<std::string> args2 = Utility::splitStringOn(output2, FIELD_DELIMITER_CHAR);

	// Update the job.
	std::string input3 = args2[1] + fieldDelimiter + args2[3];

	std::string output3 = handler.handleRequest("udjb", "", input3, nullptr);

	std::vector<std::string> args3 = Utility::splitStringOn(output3, ENTRY_DELIMITER_CHAR);

	// Check if the output is as expected.
	EXPECT_THAT(output3, testing::StartsWith(HTTPStatusCodes::success("")));
	EXPECT_GT(std::stoll(args3[1]), std::stoll(args2[3]));

	// Check if the job was updated.
	Job job = jddatabase.getCurrentJob(args2[1]);

	EXPECT_EQ(job.jobid, args2[1]);
	ASSERT_EQ(job.time, std::stoll(args3[1]));
}

// Test if sending an old job is does not update the current job.
TEST(JobDatabaseIntegrationTest, UpdateJobRequestOldJob)
{
	// Set up the test.
	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);

	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus(nullptr);
	handler.initialize(&database, &jddatabase, &raftConsensus, nullptr, TEST_IP, TEST_PORT);

	raftConsensus.start(&handler, {}, true);

	// First take the crawl job.
	std::string input = "";
	std::string output = handler.handleRequest("gtjb", "", input, nullptr);

	// Take an actual job.
	std::string output2 = handler.handleRequest("gtjb", "", input, nullptr);

	std::vector<std::string> args2 = Utility::splitStringOn(output2, FIELD_DELIMITER_CHAR);

	// Update an "older" version of the job.
	std::string input3 = args2[1] + fieldDelimiter + std::to_string(std::stoll(args2[3]) - 10);

	std::string output3 = handler.handleRequest("udjb", "", input3, nullptr);

	// Check if the output is as expected.
	EXPECT_EQ(output3, HTTPStatusCodes::clientError("Job not currently expected."));
	
	// Check if the job was not updated.
	Job job = jddatabase.getCurrentJob(args2[1]);

	EXPECT_EQ(job.jobid, args2[1]);
	ASSERT_EQ(job.time, std::stoll(args2[3]));
}

// Test if finishing a job is handled succesfully.
TEST(JobDatabaseIntegrationTest, FinishJobRequest)
{
	// Set up the test.
	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);

	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	MockStatistics stats;
	RAFTConsensus raftConsensus(nullptr);
	handler.initialize(&database, &jddatabase, &raftConsensus, &stats, TEST_IP, TEST_PORT);

	raftConsensus.start(&handler, {}, true);

	// First take the crawl job.
	std::string input = "";
	std::string output = handler.handleRequest("gtjb", "", input, nullptr);

	// Take an actual job.
	std::string output2 = handler.handleRequest("gtjb", "", input, nullptr);
	ASSERT_THAT(output2, testing::StartsWith(HTTPStatusCodes::success("")));

	std::vector<std::string> args2 = Utility::splitStringOn(output2, FIELD_DELIMITER_CHAR);

	// Finish the job.
	std::string input3 = args2[1] + fieldDelimiter + args2[3] + fieldDelimiter + "0" + fieldDelimiter + "Success.";

	std::string output3 = handler.handleRequest("fnjb", "", input3, nullptr);

	// Check if the output is as expected.
	EXPECT_EQ(output3, HTTPStatusCodes::success("Job finished succesfully."));

	// Check if the job was removed from current jobs.
	Job job = jddatabase.getCurrentJob(args2[1]);

	ASSERT_EQ(job.jobid, "");
}

// Test if failing a job is handled succesfully.
TEST(JobDatabaseIntegrationTest, FinishJobRequestFailure)
{
	// Set up the test.
	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);

	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	MockStatistics stats;
	RAFTConsensus raftConsensus(nullptr);
	handler.initialize(&database, &jddatabase, &raftConsensus, &stats, TEST_IP, TEST_PORT);

	raftConsensus.start(&handler, {}, true);

	// First take the crawl job.
	std::string input = "";
	std::string output = handler.handleRequest("gtjb", "", input, nullptr);

	// Take an actual job.
	std::string output2 = handler.handleRequest("gtjb", "", input, nullptr);
	ASSERT_THAT(output2, testing::StartsWith(HTTPStatusCodes::success("")));

	std::vector<std::string> args2 = Utility::splitStringOn(output2, FIELD_DELIMITER_CHAR);

	// Finish the job.
	std::string input3 = args2[1] + fieldDelimiter + args2[3] + fieldDelimiter + "10" + fieldDelimiter + "Project already known.";

	std::string output3 = handler.handleRequest("fnjb", "", input3, nullptr);

	// Check if the output is as expected.
	EXPECT_EQ(output3, HTTPStatusCodes::success("Job failed succesfully."));

	// Check if the job was removed from current jobs.
	Job job = jddatabase.getCurrentJob(args2[1]);

	ASSERT_EQ(job.jobid, "");
}

// Test if finishing an old job is handled correctly.
TEST(JobDatabaseIntegrationTest, FinishJobRequestOldJob)
{
	// Set up the test.
	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);

	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	MockStatistics stats;
	RAFTConsensus raftConsensus(nullptr);
	handler.initialize(&database, &jddatabase, &raftConsensus, &stats, TEST_IP, TEST_PORT);

	raftConsensus.start(&handler, {}, true);

	// First take the crawl job.
	std::string input = "";
	std::string output = handler.handleRequest("gtjb", "", input, nullptr);

	// Take an actual job.
	std::string output2 = handler.handleRequest("gtjb", "", input, nullptr);
	ASSERT_THAT(output2, testing::StartsWith(HTTPStatusCodes::success("")));

	std::vector<std::string> args2 = Utility::splitStringOn(output2, FIELD_DELIMITER_CHAR);

	// Finish the "older" job.
	std::string input3 = args2[1] + fieldDelimiter + std::to_string(std::stoll(args2[3]) - 10) + fieldDelimiter + "0" + fieldDelimiter + "Success.";

	std::string output3 = handler.handleRequest("fnjb", "", input3, nullptr);

	// Check if the output is as expected.
	EXPECT_EQ(output3, HTTPStatusCodes::clientError("Job not currently expected."));

	// Check if the job was not removed from current jobs.
	Job job = jddatabase.getCurrentJob(args2[1]);

	ASSERT_NE(job.jobid, "");
}

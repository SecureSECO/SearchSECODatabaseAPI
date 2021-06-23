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
	RAFTConsensus raftConsensus;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, &raftConsensus, TEST_IP, TEST_PORT);

	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);

	std::string input = "";
	std::string expectedOutput = "Crawl" + fieldDelimiter + "0";

	// Check if the first output is as expected.
	std::string output = handler.handleRequest("gtjb", input, nullptr);
	ASSERT_EQ(output, HTTPStatusCodes::success(expectedOutput));

	std::string expectedOutput2 = "Spider" + fieldDelimiter + "https://github.com/caged/microsis";

	// Check if the second output is as expected.
	std::string output2 = handler.handleRequest("gtjb", input, nullptr);
	ASSERT_EQ(output2, HTTPStatusCodes::success(expectedOutput2));
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
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, TEST_IP, TEST_PORT);

	std::string input = "https://github.com/mcostalba/Stockfish" + fieldDelimiter + "10";
	std::string output = handler.handleRequest("upjb", input, nullptr);
	JobRequestHandler *jhandler = new JobRequestHandler(&raftConsensus, &handler, &jddatabase, TEST_IP, TEST_PORT);
	int jobs = jhandler->numberOfJobs;
	ASSERT_EQ(jobs, 3);

	std::string input2 = "";
	std::string expectedOutput2 = "Crawl" + fieldDelimiter + "0";

	// Check if the first output is as expected.
	std::string output2 = handler.handleRequest("gtjb", input2, nullptr);
	ASSERT_EQ(output2, HTTPStatusCodes::success(expectedOutput2));

	std::string expectedOutput3 = "Spider" + fieldDelimiter + "https://github.com/mcostalba/Stockfish";

	// Check if the second output is as expected.
	std::string output3 = handler.handleRequest("gtjb", input2, nullptr);
	ASSERT_EQ(output3, HTTPStatusCodes::success(expectedOutput3));
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
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, TEST_IP, TEST_PORT);

	std::string input = "https://github.com/HackerPoet/Chaos-Equations" + fieldDelimiter + "42" + entryDelimiter +
						"https://github.com/Yiziwinnie/Home-Depot" + fieldDelimiter + "69";
	std::string output = handler.handleRequest("upjb", input, nullptr);
	JobRequestHandler *jhandler = new JobRequestHandler(&raftConsensus, &handler, &jddatabase, TEST_IP, TEST_PORT);
	int jobs = jhandler->numberOfJobs;
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
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, TEST_IP, TEST_PORT);

	std::string input =
		"100" + entryDelimiter + "https://github.com/Yiziwinnie/Bike-Sharing-in-Boston" + fieldDelimiter + "420";
	std::string output = handler.handleRequest("upcd", input, nullptr);

	std::string input2 = "";
	std::string output2 = handler.handleRequest("gtjb", input2, nullptr);
	std::string expectedOutput2 = "Crawl" + fieldDelimiter + "100";

	// Check if the output is as expected.
	ASSERT_EQ(output2, HTTPStatusCodes::success(expectedOutput2));
}

/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "RequestHandler.h"
#include "DatabaseHandler.h"
#include "DatabaseConnection.h"
#include "HTTPStatus.h"
#include "RAFTConsensus.h"
#include "Utility.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

// Test if first crawl is returned and then the correct job.
TEST(JobDatabaseIntegrationTest, GetJobRequest)
{
	// Set up:
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, "127.0.0.1", 9042);

	std::string input = "";

	std::vector<char> expectedChars = {};
	Utility::appendBy(expectedChars, {"Crawl", "0"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	expectedChars.pop_back();
	std::string expectedOutput(expectedChars.begin(), expectedChars.end());

	// Test:
	std::string output = handler.handleRequest("gtjb", input, nullptr);
	ASSERT_EQ(output, HTTPStatusCodes::success(expectedOutput));

	expectedChars = {};
	Utility::appendBy(expectedChars, {"Spider", "https://github.com/caged/microsis"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	expectedChars.pop_back();
	std::string expectedOutput2(expectedChars.begin(), expectedChars.end());

	std::string output2 = handler.handleRequest("gtjb", input, nullptr);
	ASSERT_EQ(output2, HTTPStatusCodes::success(expectedOutput2));
}

// Test if job is succesfully uploaded and the numberOfJobs variable is increased
// and the job is inserted correctly in the database.
TEST(JobDatabaseIntegrationTest, UploadJobRequest)
{
	// Set up:
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, "127.0.0.1", 9042);

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars, {"https://github.com/mcostalba/Stockfish", "10"}, FIELD_DELIMITER_CHAR,
					  ENTRY_DELIMITER_CHAR);
	inputChars.pop_back();
	std::string input(inputChars.begin(), inputChars.end());

	std::string output = handler.handleRequest("upjb", input, nullptr);
	JobRequestHandler *jhandler = new JobRequestHandler(&raftConsensus, &handler, &jddatabase, "127.0.0.1", 9042);
	int jobs = jhandler->numberOfJobs;
	ASSERT_EQ(jobs, 3);

	std::string input2 = "";

	std::vector<char> expectedChars = {};
	Utility::appendBy(expectedChars, {"Crawl", "0"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	expectedChars.pop_back();
	std::string expectedOutput2(expectedChars.begin(), expectedChars.end());

	std::string output2 = handler.handleRequest("gtjb", input2, nullptr);
	ASSERT_EQ(output2, HTTPStatusCodes::success(expectedOutput2));

	expectedChars = {};
	Utility::appendBy(expectedChars, {"Spider", "https://github.com/mcostalba/Stockfish"}, FIELD_DELIMITER_CHAR,
					  ENTRY_DELIMITER_CHAR);
	expectedChars.pop_back();
	std::string expectedOutput3(expectedChars.begin(), expectedChars.end());

	std::string output3 = handler.handleRequest("gtjb", input2, nullptr);
	ASSERT_EQ(output3, HTTPStatusCodes::success(expectedOutput3));
}

// Test if multiple jobs get succesfully uploaded.
TEST(JobDatabaseIntegrationTest, UploadMulitpleJobs)
{
	// Set up:
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, "127.0.0.1", 9042);

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars, {"https://github.com/HackerPoet/Chaos-Equations", "42"}, FIELD_DELIMITER_CHAR,
					  ENTRY_DELIMITER_CHAR);
	Utility::appendBy(inputChars, {"https://github.com/Yiziwinnie/Home-Depot", "69"}, FIELD_DELIMITER_CHAR,
					  ENTRY_DELIMITER_CHAR);
	inputChars.pop_back();
	std::string input(inputChars.begin(), inputChars.end());

	std::string output = handler.handleRequest("upjb", input, nullptr);
	JobRequestHandler *jhandler = new JobRequestHandler(&raftConsensus, &handler, &jddatabase, "127.0.0.1", 9042);
	int jobs = jhandler->numberOfJobs;
	ASSERT_EQ(jobs, 4);
}

// Test if crawl data is handled succesfully.
TEST(JobDatabaseIntegrationTest, CrawlDataRequest)
{
	// Set up:
	DatabaseHandler database;
	DatabaseConnection jddatabase;
	RequestHandler handler;
	RAFTConsensus raftConsensus;
	handler.initialize(&database, &jddatabase, &raftConsensus, "127.0.0.1", 9042);

	std::vector<char> inputChars = {};
	Utility::appendBy(inputChars, "100", ENTRY_DELIMITER_CHAR);
	Utility::appendBy(inputChars, {"https://github.com/Yiziwinnie/Bike-Sharing-in-Boston", "420"}, FIELD_DELIMITER_CHAR,
					  ENTRY_DELIMITER_CHAR);
	inputChars.pop_back();
	std::string input(inputChars.begin(), inputChars.end());
	std::string output = handler.handleRequest("upcd", input, nullptr);

	std::string input2 = "";
	std::string output2 = handler.handleRequest("gtjb", input2, nullptr);

	std::vector<char> expectedChars = {};
	Utility::appendBy(expectedChars, {"Crawl", "100"}, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	expectedChars.pop_back();
	std::string expectedOutput2(expectedChars.begin(), expectedChars.end());

	ASSERT_EQ(output2, HTTPStatusCodes::success(expectedOutput2));
}

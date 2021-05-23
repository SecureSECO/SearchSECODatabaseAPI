/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "RequestHandler.h"
#include "DatabaseHandler.h"
#include "DatabaseConnection.h"
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
        std::string expectedOutput = "Crawl?0";

        //Test:
        std::string output = handler.handleRequest("gtjb", input, nullptr);
        ASSERT_EQ(output, expectedOutput);

	std::string expectedOutput2 = "Spider?https://github.com/caged/microsis";

	handler.initialize(&database, &jddatabase, &raftConsensus, "127.0.0.1", 9042);
	std::string output2 = handler.handleRequest("gtjb", input, nullptr);
	ASSERT_EQ(output2, expectedOutput2);
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

        std::string input = "https://github.com/mcostalba/Stockfish?1";
        std::string output = handler.handleRequest("upjb", input, nullptr);
	JobRequestHandler *jhandler = new JobRequestHandler(&raftConsensus, &handler, &jddatabase, "127.0.0.1", 9042);
	int jobs = jhandler->numberOfJobs;
        ASSERT_EQ(jobs, 4);

	std::string input2 = "";
        std::string expectedOutput2 = "Crawl?0";

        std::string output2 = handler.handleRequest("gtjb", input, nullptr);
        ASSERT_EQ(output2, expectedOutput2);

        std::string expectedOutput3 = "Spider?https://github.com/mcostalba/Stockfish";

        std::string output3 = handler.handleRequest("gtjb", input, nullptr);
        ASSERT_EQ(output3, expectedOutput3);
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

        std::string input = "https://github.com/HackerPoet/Chaos-Equations?42\nhttps://github.com/Yiziwinnie/Home-Depot?69";
        std::string output = handler.handleRequest("upjb", input, nullptr);
	JobRequestHandler *jhandler = new JobRequestHandler(&raftConsensus, &handler, &jddatabase, "127.0.0.1", 9042);
	int jobs = jhandler->numberOfJobs;
        ASSERT_EQ(jobs, 6);
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

        std::string input = "100\nhttps://github.com/Yiziwinnie/Bike-Sharing-in-Boston?420";
        std::string output = handler.handleRequest("upcd", input, nullptr);
	JobRequestHandler *jhandler = new JobRequestHandler(&raftConsensus, &handler, &jddatabase, "127.0.0.1", 9042);
	int jobs = jhandler->numberOfJobs;
	int id = jhandler->crawlId;
        ASSERT_EQ(jobs, 7);
        ASSERT_EQ(id, 100);
}

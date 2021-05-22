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
}

TEST(JobDatabaseIntegrationTest, UploadJobRequest)
{
        // Set up:
        DatabaseHandler database;
        DatabaseConnection jddatabase;
        RequestHandler handler;
        RAFTConsensus raftConsensus;
	JobRequestHandler *jhandler = new JobRequestHandler(&raftConsensus, &handler, &jddatabase, "127.0.0.1", 9042);
        handler.initialize(&database, &jddatabase, &raftConsensus, "127.0.0.1", 9042);

        std::string input = "https://github.com/mcostalba/Stockfish?10";
	int jobs = jhandler->numberOfJobs;
        std::string output = handler.handleRequest("upjb", input, nullptr);
        ASSERT_EQ(jobs, 4);
}

TEST(JobDatabaseIntegrationTest, UploadMulitpleJobs)
{
        // Set up:
        DatabaseHandler database;
        DatabaseConnection jddatabase;
        RequestHandler handler;
        RAFTConsensus raftConsensus;
	JobRequestHandler *jhandler = new JobRequestHandler(&raftConsensus, &handler, &jddatabase, "127.0.0.1", 9042);
        handler.initialize(&database, &jddatabase, &raftConsensus, "127.0.0.1", 9042);

        std::string input = "https://github.com/HackerPoet/Chaos-Equations?42\nhttps://github.com/Yiziwinnie/Home-Depot?69";
	int jobs = jhandler->numberOfJobs;
        std::string output = handler.handleRequest("upjb", input, nullptr);
        ASSERT_EQ(jobs, 6);
}

TEST(JobDatabaseIntegrationTest, CrawlDataRequest)
{
        // Set up:
        DatabaseHandler database;
        DatabaseConnection jddatabase;
        RequestHandler handler;
        RAFTConsensus raftConsensus;
	JobRequestHandler *jhandler = new JobRequestHandler(&raftConsensus, &handler, &jddatabase, "127.0.0.1", 9042);
        handler.initialize(&database, &jddatabase, &raftConsensus, "127.0.0.1", 9042);

        std::string input = "100\nhttps://github.com/Yiziwinnie/Bike-Sharing-in-Boston?420";
	int jobs = jhandler->numberOfJobs;
	int id = jhandler->crawlId;
        std::string output = handler.handleRequest("upcd", input, nullptr);
        ASSERT_EQ(jobs, 7);
        ASSERT_EQ(id, 100);
}

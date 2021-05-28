/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "ConnectionHandler.h"
#include "Database-API.h"
#include "DatabaseHandler.h"
#include "ConnectionHandler.h"
#include "RAFTConsensus.h"
#include "Networking.h"

#include <gtest/gtest.h>
#include <thread>

#define TESTIP "127.0.0.1"
#define TESTPORT 9042
#define TESTCONNECTPORT 9043

TEST(ConnectionHandlerIntegrationTests, basic_request)
{
    // Set up.
	RAFTConsensus raft;
	ConnectionHandler listen;
	DatabaseHandler databaseHandler;
	DatabaseConnection databaseConnection;

	std::thread* t = new std::thread(&ConnectionHandler::startListen, &listen, &databaseHandler, &databaseConnection, &raft, TESTCONNECTPORT, TESTIP, TESTPORT);
    usleep(500000); // Just to make sure the listner has started.

    NetworkHandler* n = NetworkHandler::createHandler();
    n->openConnection("127.0.0.1", std::to_string(TESTCONNECTPORT));
    n->sendData("chck33\n");
    n->sendData("2c7f46d4f57cf9e66b03213358c7ddb5\n");
    std::string result = n->receiveData();
	const std::string expectedOutput = "2c7f46d4f57cf9e66b03213358c7ddb5?1?5000000000000?M1?P1/M1.cpp?1?1?"
										"68bd2db6-fe91-47d2-a134-cf82b104f547";
    
    ASSERT_EQ(result, HTTPStatusCodes::success(expectedOutput));
    delete n;
    delete t;
}
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
#include "RequestHandlerMock.cpp"

#include <gtest/gtest.h>
#include <thread>

#define TESTCONNECTPORT 9043

MATCHER_P(notnullptrMatcher, request, "") 
{
	return arg != nullptr;
}

TEST(ConnectionHandlerIntegrationTests, basic_request)
{
	// Set up.
	RequestHandlerMock handler; 

	const std::string input = "2c7f46d4f57cf9e66b03213358c7ddb5\n";

	const std::string expectedOutput = "2c7f46d4f57cf9e66b03213358c7ddb5?1?5000000000000?M1?P1/M1.cpp?1?1?"
										"68bd2db6-fe91-47d2-a134-cf82b104f547\n";
	// Mock expectations
	EXPECT_CALL(handler, handleRequest("chck", input, notnullptrMatcher(nullptr))).Times(1).WillOnce(testing::Return(expectedOutput));

	ConnectionHandler listen;

	std::thread* t = new std::thread(&ConnectionHandler::startListen, &listen, nullptr, nullptr, nullptr, TESTCONNECTPORT, &handler);
	usleep(500000); // Just to make sure the listner has started.

	NetworkHandler* n = NetworkHandler::createHandler();
	n->openConnection("127.0.0.1", std::to_string(TESTCONNECTPORT));
	n->sendData("chck" + std::to_string(input.size()) +"\n");
	n->sendData(input);
	std::string result = n->receiveData(false);

	
	ASSERT_EQ(result, expectedOutput);
	delete n;
}

TEST(ConnectionHandlerIntegrationTests, basic_in_chunks)
{
	// Set up.
	RequestHandlerMock handler; 

	const std::string inputPart1 = "2c7f46d4f57cf9e66";
	const std::string inputPart2 = "b03213358c7ddb5\n";

	const std::string expectedOutput = "2c7f46d4f57cf9e66b03213358c7ddb5?1?5000000000000?M1?P1/M1.cpp?1?1?"
										"68bd2db6-fe91-47d2-a134-cf82b104f547\n";
	// Mock expectations
	EXPECT_CALL(handler, handleRequest("chck", inputPart1 + inputPart2, notnullptrMatcher(nullptr))).Times(1).WillOnce(testing::Return(expectedOutput));

	ConnectionHandler listen;

	std::thread* t = new std::thread(&ConnectionHandler::startListen, &listen, nullptr, nullptr, nullptr, TESTCONNECTPORT, &handler);
	usleep(500000); // Just to make sure the listner has started.

	NetworkHandler* n = NetworkHandler::createHandler();
	n->openConnection("127.0.0.1", std::to_string(TESTCONNECTPORT));
	n->sendData("chck" + std::to_string(input.size()) +"\n");
	n->sendData(inputPart1);
	usleep(500000);
	n->sendData(inputPart2);
	std::string result = n->receiveData(false);

	
	ASSERT_EQ(result, expectedOutput);
	delete n;
}

TEST(ConnectionHandlerIntegrationTests, to_big_request)
{
	// Set up.
	RequestHandlerMock handler; 

	const std::string input = "2c7f46d4f57cf9e66b03213358c7ddb5\n";

	const std::string expectedOutput = "Request body larger than expected.";
	
	ConnectionHandler listen;

	std::thread* t = new std::thread(&ConnectionHandler::startListen, &listen, nullptr, nullptr, nullptr, TESTCONNECTPORT, &handler);
	usleep(500000); // Just to make sure the listner has started.

	NetworkHandler* n = NetworkHandler::createHandler();
	n->openConnection("127.0.0.1", std::to_string(TESTCONNECTPORT));
	n->sendData("chck" + std::to_string(input.size() - 10) +"\n");
	n->sendData(input);
	std::string result = n->receiveData(false);

	
	ASSERT_EQ(result, expectedOutput);
	delete n;
}

TEST(ConnectionHandlerIntegrationTests, invalid)
{
	// Set up.
	RequestHandlerMock handler; 

	const std::string input = "2c7f46d4f57cf9e66b03213358c7ddb5\n";

	const std::string expectedOutput = "Error parsing command.";
	
	ConnectionHandler listen;

	std::thread* t = new std::thread(&ConnectionHandler::startListen, &listen, nullptr, nullptr, nullptr, TESTCONNECTPORT, &handler);
	usleep(500000); // Just to make sure the listner has started.

	NetworkHandler* n = NetworkHandler::createHandler();
	n->openConnection("127.0.0.1", std::to_string(TESTCONNECTPORT));
	n->sendData("chckk\n");
	n->sendData(input);
	std::string result = n->receiveData(false);

	
	ASSERT_EQ(result, expectedOutput);
	delete n;
}
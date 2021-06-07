/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "ConnectionHandler.h"
#include "Database-API.h"
#include "DatabaseHandler.h"
#include "Definitions.h"
#include "ConnectionHandler.h"
#include "Networking.h"
#include "RAFTConsensus.h"
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
	std::string entryDelimiter(1, ENTRY_DELIMITER_CHAR);
	// Set up.
	RequestHandlerMock handler; 



	const std::string input = "2c7f46d4f57cf9e66b03213358c7ddb5" + entryDelimiter;

	
	std::vector<char> outputChars = {};

	Utility::appendBy(outputChars, {"2c7f46d4f57cf9e66b03213358c7ddb5", "1", "5000000000000" "M1", "P1/M1.cpp", "1", "1",
									"68bd2db6-fe91-47d2-a134-cf82b104f547"},
					 				 FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);

	const std::string expectedOutput = std::string(outputChars.begin(), outputChars.end());
	// Mock expectations
	EXPECT_CALL(handler, handleRequest("chck", input, notnullptrMatcher(nullptr))).Times(1).WillOnce(testing::Return(expectedOutput));

	ConnectionHandler listen;

	std::thread* t = new std::thread(&ConnectionHandler::startListen, &listen, nullptr, nullptr, nullptr, TESTCONNECTPORT, &handler);
	usleep(500000); // Just to make sure the listner has started.

	NetworkHandler* n = NetworkHandler::createHandler();
	n->openConnection("127.0.0.1", std::to_string(TESTCONNECTPORT));
	n->sendData("chck" + std::to_string(input.size()) + entryDelimiter);
	n->sendData(input);
	std::string result = n->receiveData(false);

	
	ASSERT_EQ(result, expectedOutput);
	delete n;
}

TEST(ConnectionHandlerIntegrationTests, basic_in_chunks)
{
	std::string entryDelimiter(1, ENTRY_DELIMITER_CHAR);
	// Set up.
	RequestHandlerMock handler; 

	const std::string input = "2c7f46d4f57cf9e66b03213358c7ddb5" + entryDelimiter;

	std::vector<char> outputChars = {};

	Utility::appendBy(outputChars, {"2c7f46d4f57cf9e66b03213358c7ddb5","1","5000000000000","M1","P1/M1.cpp","1","1",
										"68bd2db6-fe91-47d2-a134-cf82b104f547"},
					 				 FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	const std::string expectedOutput = std::string(outputChars.begin(), outputChars.end());
	// Mock expectations
	EXPECT_CALL(handler, handleRequest("chck", input, notnullptrMatcher(nullptr))).Times(1).WillOnce(testing::Return(expectedOutput));

	ConnectionHandler listen;

	std::thread* t = new std::thread(&ConnectionHandler::startListen, &listen, nullptr, nullptr, nullptr, TESTCONNECTPORT, &handler);
	usleep(500000); // Just to make sure the listner has started.

	NetworkHandler* n = NetworkHandler::createHandler();
	n->openConnection("127.0.0.1", std::to_string(TESTCONNECTPORT));
	n->sendData("chck" + std::to_string(input.size()) + entryDelimiter);
	n->sendData(input.substr(0, 12));
	usleep(500000);
	n->sendData(input.substr(12));
	std::string result = n->receiveData(false);

	
	ASSERT_EQ(result, expectedOutput);
	delete n;
}

TEST(ConnectionHandlerIntegrationTests, to_big_request)
{
	std::string entryDelimiter(1, ENTRY_DELIMITER_CHAR);
	// Set up.
	RequestHandlerMock handler; 

	const std::string input = "2c7f46d4f57cf9e66b03213358c7ddb5" + entryDelimiter;

	const std::string expectedOutput = "Request body larger than expected.";
	
	ConnectionHandler listen;

	std::thread* t = new std::thread(&ConnectionHandler::startListen, &listen, nullptr, nullptr, nullptr, TESTCONNECTPORT, &handler);
	usleep(500000); // Just to make sure the listner has started.

	NetworkHandler* n = NetworkHandler::createHandler();
	n->openConnection("127.0.0.1", std::to_string(TESTCONNECTPORT));
	n->sendData("chck" + std::to_string(input.size() - 10) + entryDelimiter);
	n->sendData(input);
	std::string result = n->receiveData(false);

	
	ASSERT_EQ(result, expectedOutput);
	delete n;
}

TEST(ConnectionHandlerIntegrationTests, invalid)
{
	std::string entryDelimiter(1, ENTRY_DELIMITER_CHAR);
	// Set up.
	RequestHandlerMock handler; 

	const std::string input = "2c7f46d4f57cf9e66b03213358c7ddb5" + entryDelimiter;

	const std::string expectedOutput = "Error parsing command.";
	
	ConnectionHandler listen;

	std::thread* t = new std::thread(&ConnectionHandler::startListen, &listen, nullptr, nullptr, nullptr, TESTCONNECTPORT, &handler);
	usleep(500000); // Just to make sure the listner has started.

	NetworkHandler* n = NetworkHandler::createHandler();
	n->openConnection("127.0.0.1", std::to_string(TESTCONNECTPORT));
	n->sendData("chckk" + entryDelimiter);
	n->sendData(input);
	std::string result = n->receiveData(false);

	
	ASSERT_EQ(result, expectedOutput);
	delete n;
}
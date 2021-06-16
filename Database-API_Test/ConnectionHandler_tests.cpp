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

#define TESTIP "127.0.0.1"

MATCHER_P(notnullptrMatcher, request, "") 
{
	return arg != nullptr;
}

TEST(ConnectionHandlerIntegrationTests, basic_request)
{
	const int TESTCONNECTPORT = 9044;
	std::string entryDelimiter(1, ENTRY_DELIMITER_CHAR);
	// Set up.
	RequestHandlerMock handler; 
	
	const std::string input = "2c7f46d4f57cf9e66b03213358c7ddb5" + entryDelimiter;
	
	std::vector<char> outputChars = {};
	Utility::appendBy(outputChars, {"2c7f46d4f57cf9e66b03213358c7ddb5", "1", "5000000000000", "M1", "P1/M1.cpp", "1", "1",
									"68bd2db6-fe91-47d2-a134-cf82b104f547"},
					 				FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);

	const std::string expectedOutput = std::string(outputChars.begin(), outputChars.end());
	// Mock expectations.
	EXPECT_CALL(handler, handleRequest("chck", input, notnullptrMatcher(nullptr))).Times(1).WillOnce(testing::Return(expectedOutput));

	ConnectionHandler listen;

	std::thread* thread = new std::thread(&ConnectionHandler::startListen, &listen, nullptr, nullptr, nullptr, TESTCONNECTPORT, &handler);
	usleep(500000); // Just to make sure the listner has started.

	NetworkHandler* networkHandler = NetworkHandler::createHandler();
	networkHandler->openConnection(TESTIP, std::to_string(TESTCONNECTPORT));
	networkHandler->sendData("chck" + std::to_string(input.size()) + entryDelimiter);
	networkHandler->sendData(input);
	std::string result = networkHandler->receiveData(false);

	
	ASSERT_EQ(result, expectedOutput);
	delete networkHandler;
}

TEST(ConnectionHandlerIntegrationTests, basic_in_chunks)
{
	const int TESTCONNECTPORT = 9045;
	std::string entryDelimiter(1, ENTRY_DELIMITER_CHAR);
	// Set up.
	RequestHandlerMock handler; 

	const std::string input = "2c7f46d4f57cf9e66b03213358c7ddb5" + entryDelimiter;

	std::vector<char> outputChars = {};

	Utility::appendBy(outputChars, {"2c7f46d4f57cf9e66b03213358c7ddb5","1","5000000000000","M1","P1/M1.cpp","1","1",
									"68bd2db6-fe91-47d2-a134-cf82b104f547"},
					 				FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	const std::string expectedOutput = std::string(outputChars.begin(), outputChars.end());
	// Mock expectations.
	EXPECT_CALL(handler, handleRequest("chck", input, notnullptrMatcher(nullptr))).Times(1).WillOnce(testing::Return(expectedOutput));

	ConnectionHandler listen;

	std::thread* thread = new std::thread(&ConnectionHandler::startListen, &listen, nullptr, nullptr, nullptr, TESTCONNECTPORT, &handler);
	usleep(500000); // Just to make sure the listner has started.

	NetworkHandler* networkHandler = NetworkHandler::createHandler();
	networkHandler->openConnection(TESTIP, std::to_string(TESTCONNECTPORT));
	networkHandler->sendData("chck" + std::to_string(input.size()) + entryDelimiter);
	networkHandler->sendData(input.substr(0, 12));
	usleep(500000);
	networkHandler->sendData(input.substr(12));
	std::string result = networkHandler->receiveData(false);

	
	ASSERT_EQ(result, expectedOutput);
	delete networkHandler;
}

TEST(ConnectionHandlerIntegrationTests, too_big_request)
{
	const int TESTCONNECTPORT = 9046;
	std::string entryDelimiter(1, ENTRY_DELIMITER_CHAR);
	// Set up.
	RequestHandlerMock handler; 

	const std::string input = "2c7f46d4f57cf9e66b03213358c7ddb5" + entryDelimiter;

	const std::string expectedOutput = "Request body larger than expected.";
	
	ConnectionHandler listen;

	std::thread* thread = new std::thread(&ConnectionHandler::startListen, &listen, nullptr, nullptr, nullptr, TESTCONNECTPORT, &handler);
	usleep(500000); // Just to make sure the listner has started.

	NetworkHandler* networkHandler = NetworkHandler::createHandler();
	networkHandler->openConnection(TESTIP, std::to_string(TESTCONNECTPORT));
	networkHandler->sendData("chck" + std::to_string(input.size() - 10) + entryDelimiter);
	networkHandler->sendData(input);
	std::string result = networkHandler->receiveData(false);

	
	ASSERT_EQ(result, expectedOutput);
	delete networkHandler;
}

TEST(ConnectionHandlerIntegrationTests, invalid)
{
	const int TESTCONNECTPORT = 9047;
	std::string entryDelimiter(1, ENTRY_DELIMITER_CHAR);
	// Set up.
	RequestHandlerMock handler; 

	const std::string input = "2c7f46d4f57cf9e66b03213358c7ddb5" + entryDelimiter;

	const std::string expectedOutput = "Error parsing command.";
	
	ConnectionHandler listen;

	std::thread* thread = new std::thread(&ConnectionHandler::startListen, &listen, nullptr, nullptr, nullptr, TESTCONNECTPORT, &handler);
	usleep(500000); // Just to make sure the listner has started.

	NetworkHandler* networkHandler = NetworkHandler::createHandler();
	networkHandler->openConnection(TESTIP, std::to_string(TESTCONNECTPORT));
	networkHandler->sendData("chckk" + entryDelimiter);
	networkHandler->sendData(input);
	std::string result = networkHandler->receiveData(false);

	
	ASSERT_EQ(result, expectedOutput);
	delete networkHandler;
}
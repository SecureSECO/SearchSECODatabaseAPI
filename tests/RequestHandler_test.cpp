/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include "HTTPStatus.h"
#include "JDDatabaseMock.cpp"
#include "RAFTConsensus.h"
#include "DatabaseConnection.h"

#include <gtest/gtest.h>

// Tests if the RequestHandler requests to connect to the database when initialized.
TEST(GeneralTest, InitializeTest)
{
	// Set up the test.
	errno = 0;

	RequestHandler handler;
	MockDatabase database;
	MockJDDatabase jddatabase;
	EXPECT_CALL(database, connect(IP, DBPORT)).Times(1);
	errno = 0;
	handler.initialize(&database, &jddatabase, nullptr, nullptr);
}

// Tests if the RequestHandler correctly responds to an unknown request.
TEST(GeneralTest, UnknownRequest)
{
	// Set up the test.
	errno = 0;

	RequestHandler handler;

	EXPECT_EQ(handler.handleRequest("kill", "", "", nullptr), HTTPStatusCodes::clientError("Unknown request type."));
}

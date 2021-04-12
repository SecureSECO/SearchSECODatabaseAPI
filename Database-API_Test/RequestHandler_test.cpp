#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include <gtest/gtest.h>

// Tests if the RequestHandler requests to connect to the database when initialized.
TEST(GeneralTest, InitializeTest){
	RequestHandler handler;
	MockDatabase database;
	EXPECT_CALL(database, connect())
		.Times(1);
	handler.initialize(&database);
}

// Tests if the RequestHandler correctly responds to an unknown request.
TEST(SimpleTest, BasicAssertions){
	RequestHandler handler;
	EXPECT_EQ(handler.handleRequest("kill", ""), "Your input is not recognised.");
}

#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include <gtest/gtest.h>

TEST(GeneralTest, InitializeTest){
	RequestHandler handler;
	MockDatabase database;
	EXPECT_CALL(database, Connect())
		.Times(1);
	handler.initialize(&database);
}

TEST(SimpleTest, BasicAssertions){
	RequestHandler handler;
	EXPECT_EQ(handler.handleRequest("kill", ""), "Your input is not recognised.");
}

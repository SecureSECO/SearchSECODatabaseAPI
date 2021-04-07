#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include <gtest/gtest.h>

TEST(SimpleTest, BasicAssertions){
	RequestHandler handler;
	EXPECT_EQ(handler.handleRequest("kill", ""), "Your input is not recognised.");
}

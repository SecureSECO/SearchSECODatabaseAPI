#include "RequestHandler.h"
#include <gtest/gtest.h>

TEST(SimpleTest, BasicAssertions){
	RequestHandler handler;
	EXPECT_EQ(handler.handleRequest("kill", ""), "Your input is not recognised.");
}

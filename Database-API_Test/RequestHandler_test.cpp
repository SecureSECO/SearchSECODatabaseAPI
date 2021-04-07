#include "RequestHandler.h"
#include <gtest/gtest.h>

TEST(SimpleTest, BasicAssertions){
	RequestHandler handler;
	EXPECT_EQ(handler.handleUnknownRequest(), "Your input is not recognised.");
}

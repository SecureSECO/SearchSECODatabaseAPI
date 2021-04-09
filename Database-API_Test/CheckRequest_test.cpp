/*This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
 Copyright Utrecht University(Department of Informationand Computing Sciences)*/

#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include <gtest/gtest.h>

TEST(CheckRequestTests, Verify_HandleRequest)
{
	MockDatabase database;
	RequestHandler handler;
	std::string output = "2c7f46d4f57cf9e66b03213358c7ddb5|TestMethod|Test1/Test2/TestFile.cpp|69|1|f1a028d7-3845-41df-bec1-2e16c49e4c35";
	MethodOut testMethod;
	testMethod.hash = "2c7f46d4f57cf9e66b03213358c7ddb5";
	testMethod.projectID = 1;
	testMethod.version = 2;
	testMethod.methodName = "TestMethod";
	testMethod.fileLocation = "Test1/Test2/TestFile.cpp";
	testMethod.lineNumber = 69;
	testMethod.authorIDs = { "f1a028d7-3845-41df-bec1-2e16c49e4c35" };
	std::vector<MethodOut> v;
	v.push_back(testMethod);

	EXPECT_CALL(database,HashToMethods("2c7f46d4f57cf9e66b03213358c7ddb5")).WillOnce(testing::Return(v));
	handler.handleRequest("chck", "2c7f46d4f57cf9e66b03213358c7ddb5 \n");
	ASSERT_EQ(handler.handleRequest("chck", "2c7f46d4f57cf9e66b03213358c7ddb5 \n"), output);
}

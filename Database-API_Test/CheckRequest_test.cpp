/*This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)*/

#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include "JDDatabaseMock.cpp"
#include <gtest/gtest.h>

std::string output1 = "2c7f46d4f57cf9e66b03213358c7ddb5?1?2?TestMethod1?Test1/Test2/TestFile1.cpp?69?1?"
					  "f1a028d7-3845-41df-bec1-2e16c49e4c35\n";
MethodOut testMethod1 = { .hash = "2c7f46d4f57cf9e66b03213358c7ddb5", .projectID = 1, .version = 2,
						  .methodName = "TestMethod1", .fileLocation = "Test1/Test2/TestFile1.cpp", .lineNumber = 69,
						  .authorIDs = { "f1a028d7-3845-41df-bec1-2e16c49e4c35" } };
std::string output2 = "06f73d7ab46184c55bf4742b9428a4c0?3?4?TestMethod2?Test3/Test4/TestFile2.cpp?42?2?"
					  "f1a028d7-3845-41df-bec1-2e16c49e4c35?8b55fa97-5442-48f7-969c-793664388264\n";
MethodOut testMethod2 = { .hash = "06f73d7ab46184c55bf4742b9428a4c0", .projectID = 3, .version = 4,
						  .methodName = "TestMethod2", .fileLocation = "Test3/Test4/TestFile2.cpp", .lineNumber = 42,
						  .authorIDs = { "f1a028d7-3845-41df-bec1-2e16c49e4c35",
										 "8b55fa97-5442-48f7-969c-793664388264" } };
std::string output3 = "137fed017b6159acc0af30d2c6b403a5?5?6?TestMethod3?Test5/Test6/TestFile3.cpp?420?3?"
					  "f1a028d7-3845-41df-bec1-2e16c49e4c35?fa37bbe0-ef68-4653-800c-d5bf30dcc7ef?"
					  "5a726b9e-6173-4fc1-831b-54f98f6e760c\n";
MethodOut testMethod3 = { .hash = "137fed017b6159acc0af30d2c6b403a5", .projectID = 5, .version = 6,
						  .methodName = "TestMethod3", .fileLocation = "Test5/Test6/TestFile3.cpp", .lineNumber = 420,
						  .authorIDs = { "f1a028d7-3845-41df-bec1-2e16c49e4c35",
										 "fa37bbe0-ef68-4653-800c-d5bf30dcc7ef",
										 "5a726b9e-6173-4fc1-831b-54f98f6e760c" } };
std::string output4 = "2c7f46d4f57cf9e66b03213358c7ddb5?7?8?TestMethod4?Test7/Test8/TestFile4.cpp?69?1?"
					  "f1a028d7-3845-41df-bec1-2e16c49e4c35\n";
MethodOut testMethod4 = { .hash = "2c7f46d4f57cf9e66b03213358c7ddb5", .projectID = 7, .version = 8,
			  .methodName = "TestMethod4", .fileLocation = "Test7/Test8/TestFile4.cpp", .lineNumber = 69,
			  .authorIDs = { "f1a028d7-3845-41df-bec1-2e16c49e4c35" }};
std::string output5 = "06f73d7ab46184c55bf4742b9428a4c0?9?10?TestMethod5?Test9/Test10/TestFile5.cpp?42?2?"
					  "f1a028d7-3845-41df-bec1-2e16c49e4c35?8b55fa97-5442-48f7-969c-793664388264\n";
MethodOut testMethod5 = { .hash = "06f73d7ab46184c55bf4742b9428a4c0", .projectID = 9, .version = 10,
						  .methodName = "TestMethod5", .fileLocation = "Test9/Test10/TestFile5.cpp", .lineNumber = 42,
						  .authorIDs = { "f1a028d7-3845-41df-bec1-2e16c49e4c35",
										 "8b55fa97-5442-48f7-969c-793664388264" } };

// Checks if the program works when a check request is sent providing a single hash.
TEST(CheckRequestTests, SingleHashRequest)
{
	MockDatabase database;
	MockJDDatabase jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr);
	std::vector<MethodOut> v;
	v.push_back(testMethod1);

	EXPECT_CALL(database,hashToMethods("2c7f46d4f57cf9e66b03213358c7ddb5")).WillOnce(testing::Return(v));
	std::string result = handler.handleRequest("chck", "2c7f46d4f57cf9e66b03213358c7ddb5\n", nullptr);
	EXPECT_EQ(result, output1);
}

// Checks if the program works when a check request is sent providing multiple hashes.
TEST(CheckRequestTests, MultipleHashRequest)
{
	MockDatabase database;
	MockJDDatabase jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr);
	std::vector<MethodOut> v1;
	v1.push_back(testMethod1);
	std::vector<MethodOut> v2;
	v2.push_back(testMethod2);
	std::vector<MethodOut> v3;
	v3.push_back(testMethod3);

	EXPECT_CALL(database,hashToMethods("2c7f46d4f57cf9e66b03213358c7ddb5")).WillOnce(testing::Return(v1));
	EXPECT_CALL(database,hashToMethods("06f73d7ab46184c55bf4742b9428a4c0")).WillOnce(testing::Return(v2));
	EXPECT_CALL(database,hashToMethods("137fed017b6159acc0af30d2c6b403a5")).WillOnce(testing::Return(v3));
	std::string result = handler.handleRequest("chck", "2c7f46d4f57cf9e66b03213358c7ddb5\n"
													   "06f73d7ab46184c55bf4742b9428a4c0\n"
													   "137fed017b6159acc0af30d2c6b403a5\n", nullptr);
		EXPECT_TRUE(result.find(output1) != std::string::npos);
	EXPECT_TRUE(result.find(output2) != std::string::npos);
	EXPECT_TRUE(result.find(output3) != std::string::npos);
}

// Checks if the program can successfully handle a check request with a hash which is not in the database.
TEST(CheckRequestTests, SingleHashNoMatch)
{
	MockDatabase database;
	MockJDDatabase jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr);
	std::vector<MethodOut> v;

	EXPECT_CALL(database,hashToMethods("2c7f46d4f57cf9e66b03213358c7ddb5")).WillOnce(testing::Return(v));
	std::string result = handler.handleRequest("chck", "2c7f46d4f57cf9e66b03213358c7ddb5\n", nullptr);
	EXPECT_EQ(result, "No results found.");
}

// Checks if the program can successfully handle a check request with multiple hashes, all with one match.
TEST(CheckRequestTests, MultipleHashOneMatch)
{
	MockDatabase database;
	MockJDDatabase jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr);
	std::vector<MethodOut> v;
	std::vector<MethodOut> v2;
	v.push_back(testMethod2);

	EXPECT_CALL(database,hashToMethods("2c7f46d4f57cf9e66b03213358c7ddb5")).WillOnce(testing::Return(v2));
	EXPECT_CALL(database,hashToMethods("06f73d7ab46184c55bf4742b9428a4c0")).WillOnce(testing::Return(v));
	EXPECT_CALL(database,hashToMethods("137fed017b6159acc0af30d2c6b403a5")).WillOnce(testing::Return(v2));
	std::string result = handler.handleRequest("chck", "2c7f46d4f57cf9e66b03213358c7ddb5\n"
														   "06f73d7ab46184c55bf4742b9428a4c0\n"
														   "137fed017b6159acc0af30d2c6b403a5\n", nullptr);

	EXPECT_FALSE(result.find(output1) != std::string::npos);
	EXPECT_TRUE(result.find(output2) != std::string::npos);
	EXPECT_FALSE(result.find(output3) != std::string::npos);
}

// Checks if the program can successfully handle a check request with one hash, having multiple matches.
TEST(CheckRequestTests, OneHashMultipleMatches)
{
	MockDatabase database;
	MockJDDatabase jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr);
	std::vector<MethodOut> v;
	v.push_back(testMethod1);
	v.push_back(testMethod4);

	EXPECT_CALL(database,hashToMethods("2c7f46d4f57cf9e66b03213358c7ddb5")).WillOnce(testing::Return(v));
	std::string result = handler.handleRequest("chck", "2c7f46d4f57cf9e66b03213358c7ddb5\n", nullptr);
	EXPECT_TRUE(result.find(output1) != std::string::npos);
	EXPECT_TRUE(result.find(output4) != std::string::npos);
}

// Check if the program can successfully handle a check request with multiple hashes, having multiple matches.
TEST(CheckRequestTests, MultipleHashesMultipleMatches)
{
	MockDatabase database;
	MockJDDatabase jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr);
	std::vector<MethodOut> v1;
	std::vector<MethodOut> v2;
	std::vector<MethodOut> v3;
	v1.push_back(testMethod1);
	v1.push_back(testMethod4);
	v2.push_back(testMethod2);
	v2.push_back(testMethod5);
	v3.push_back(testMethod3);

	EXPECT_CALL(database,hashToMethods("2c7f46d4f57cf9e66b03213358c7ddb5")).WillOnce(testing::Return(v1));
	EXPECT_CALL(database,hashToMethods("06f73d7ab46184c55bf4742b9428a4c0")).WillOnce(testing::Return(v2));
	EXPECT_CALL(database,hashToMethods("137fed017b6159acc0af30d2c6b403a5")).WillOnce(testing::Return(v3));
	std::string result = handler.handleRequest("chck", "2c7f46d4f57cf9e66b03213358c7ddb5\n"
							   "06f73d7ab46184c55bf4742b9428a4c0\n"
							   "137fed017b6159acc0af30d2c6b403a5\n", nullptr);

	EXPECT_TRUE(result.find(output1) != std::string::npos);
	EXPECT_TRUE(result.find(output2) != std::string::npos);
	EXPECT_TRUE(result.find(output3) != std::string::npos);
	EXPECT_TRUE(result.find(output4) != std::string::npos);
	EXPECT_TRUE(result.find(output5) != std::string::npos);
}

// Checks if the program correctly identifies an invalid hash in the input.
TEST(CheckRequestTests, InvalidHash)
{
	RequestHandler handler;
	std::string request = "hello_I'm_an_invalid_hash";

	std::string output = handler.handleRequest("chck", request, nullptr);
	ASSERT_EQ(output, "Invalid hash presented.");
}

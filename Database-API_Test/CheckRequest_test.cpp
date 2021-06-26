/*This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)*/

#include "Definitions.h"
#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include "JDDatabaseMock.cpp"
#include "HTTPStatus.h"
#include "Utility.h"

#include <gtest/gtest.h>

MethodOut testMethod1 = {.hash = "2c7f46d4f57cf9e66b03213358c7ddb5",
						 .projectID = 1,
						 .fileLocation = "Test1/Test2/TestFile1.cpp",
						 .startVersion = 2,
						 .startVersionHash = "1e134519e9365151e9fa7e4aadf981e115a74421",
						 .endVersion = 2,
						 .endVersionHash = "1e134519e9365151e9fa7e4aadf981e115a74421",
						 .methodName = "TestMethod1",
						 .lineNumber = 69,
						 .authorIDs = {"f1a028d7-3845-41df-bec1-2e16c49e4c35"},
						 .parserVersion = 1};

MethodOut testMethod2 = {.hash = "06f73d7ab46184c55bf4742b9428a4c0",
						 .projectID = 3,
						 .fileLocation = "Test3/Test4/TestFile2.cpp",
						 .startVersion = 4,
						 .startVersionHash = "1e134519e9365151e9fa7e4aadf981e115a74422",
						 .endVersion = 4,
						 .endVersionHash = "1e134519e9365151e9fa7e4aadf981e115a74422",
						 .methodName = "TestMethod2",
						 .lineNumber = 42,
						 .authorIDs = {"f1a028d7-3845-41df-bec1-2e16c49e4c35", "8b55fa97-5442-48f7-969c-793664388264"},
						 .parserVersion = 1};

MethodOut testMethod3 = {.hash = "137fed017b6159acc0af30d2c6b403a5",
						 .projectID = 5,
						 .fileLocation = "Test5/Test6/TestFile3.cpp",
						 .startVersion = 6,
						 .startVersionHash = "1e134519e9365151e9fa7e4aadf981e115a84422",
						 .endVersion = 6,
						 .endVersionHash = "1e134519e9365151e9fa7e4aadf981e115a84422",
						 .methodName = "TestMethod3",
						 .lineNumber = 420,
						 .authorIDs = {"f1a028d7-3845-41df-bec1-2e16c49e4c35", "fa37bbe0-ef68-4653-800c-d5bf30dcc7ef",
									   "5a726b9e-6173-4fc1-831b-54f98f6e760c"},
						 .parserVersion = 1};

MethodOut testMethod4 = {.hash = "2c7f46d4f57cf9e66b03213358c7ddb5",
						 .projectID = 7,
						 .fileLocation = "Test7/Test8/TestFile4.cpp",
						 .startVersion = 8,
						 .startVersionHash = "1e134519e9465151e9fa7e4aadf981e115a84422",
						 .endVersion = 8,
						 .endVersionHash = "1e134519e9465151e9fa7e4aadf981e115a84422",
						 .methodName = "TestMethod4",
						 .lineNumber = 69,
						 .authorIDs = {"f1a028d7-3845-41df-bec1-2e16c49e4c35"},
						 .parserVersion = 1};

MethodOut testMethod5 = {.hash = "06f73d7ab46184c55bf4742b9428a4c0",
						 .projectID = 9,
						 .fileLocation = "Test9/Test10/TestFile5.cpp",
						 .startVersion = 10,
						 .startVersionHash = "1e134519e9765151e9fa7e4aadf981e115a84422",
						 .endVersion = 10,
						 .endVersionHash = "1e134519e9765151e9fa7e4aadf981e115a84422",
						 .methodName = "TestMethod5",
						 .lineNumber = 42,
						 .authorIDs = {"f1a028d7-3845-41df-bec1-2e16c49e4c35", "8b55fa97-5442-48f7-969c-793664388264"},
						 .parserVersion = 1};

// Checks if the program works when a check request is sent providing a single hash.
TEST(CheckRequestTests, SingleHashRequest)
{
	// Set up the test.
	errno = 0;

	MockDatabase database;
	MockJDDatabase jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr);
	std::vector<MethodOut> v;
	v.push_back(testMethod1);

	std::vector<char> outputChars1 = {};
	Utility::appendBy(outputChars1,
					  {"2c7f46d4f57cf9e66b03213358c7ddb5", "1", "2", "1e134519e9365151e9fa7e4aadf981e115a74421", "2",
					   "1e134519e9365151e9fa7e4aadf981e115a74421", "TestMethod1", "Test1/Test2/TestFile1.cpp", "69",
					   "1", "1", "f1a028d7-3845-41df-bec1-2e16c49e4c35"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output1(outputChars1.begin(), outputChars1.end());

	EXPECT_CALL(database,hashToMethods("2c7f46d4f57cf9e66b03213358c7ddb5")).WillOnce(testing::Return(v));

	// Check if the output is correct.
	std::string result = handler.handleRequest("chck", "2c7f46d4f57cf9e66b03213358c7ddb5", nullptr);
	EXPECT_EQ(result, HTTPStatusCodes::success(output1));

}

// Checks if the program works when a check request is sent providing multiple hashes.
TEST(CheckRequestTests, MultipleHashRequest)
{
	// Set up the test.
	errno = 0;

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

	// We expect some calls towards the database.
	EXPECT_CALL(database, hashToMethods("2c7f46d4f57cf9e66b03213358c7ddb5")).WillOnce(testing::Return(v1));
	EXPECT_CALL(database, hashToMethods("06f73d7ab46184c55bf4742b9428a4c0")).WillOnce(testing::Return(v2));
	EXPECT_CALL(database, hashToMethods("137fed017b6159acc0af30d2c6b403a5")).WillOnce(testing::Return(v3));

	std::vector<char> inputFunctionChars = {};
	Utility::appendBy(
		inputFunctionChars,
		{"2c7f46d4f57cf9e66b03213358c7ddb5", "06f73d7ab46184c55bf4742b9428a4c0", "137fed017b6159acc0af30d2c6b403a5"},
		ENTRY_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string inputFunction(inputFunctionChars.begin(), inputFunctionChars.end());
	std::string result = handler.handleRequest("chck", inputFunction, nullptr);

	std::vector<char> outputChars1 = {};
	Utility::appendBy(outputChars1,
					  {"2c7f46d4f57cf9e66b03213358c7ddb5", "1", "2", "1e134519e9365151e9fa7e4aadf981e115a74421", "2",
					   "1e134519e9365151e9fa7e4aadf981e115a74421", "TestMethod1", "Test1/Test2/TestFile1.cpp", "69",
					   "1", "1", "f1a028d7-3845-41df-bec1-2e16c49e4c35"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output1(outputChars1.begin(), outputChars1.end());
	std::vector<char> outputChars2 = {};
	Utility::appendBy(outputChars2,
					  {"06f73d7ab46184c55bf4742b9428a4c0", "3", "4", "1e134519e9365151e9fa7e4aadf981e115a74422", "4",
					   "1e134519e9365151e9fa7e4aadf981e115a74422", "TestMethod2", "Test3/Test4/TestFile2.cpp", "42",
					   "1", "2", "f1a028d7-3845-41df-bec1-2e16c49e4c35", "8b55fa97-5442-48f7-969c-793664388264"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output2(outputChars2.begin(), outputChars2.end());
	std::vector<char> outputChars3 = {};
	Utility::appendBy(outputChars3,
					  {"137fed017b6159acc0af30d2c6b403a5", "5", "6", "1e134519e9365151e9fa7e4aadf981e115a84422", "6",
					   "1e134519e9365151e9fa7e4aadf981e115a84422", "TestMethod3", "Test5/Test6/TestFile3.cpp", "420",
					   "1", "3", "f1a028d7-3845-41df-bec1-2e16c49e4c35", "fa37bbe0-ef68-4653-800c-d5bf30dcc7ef",
					   "5a726b9e-6173-4fc1-831b-54f98f6e760c"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output3(outputChars3.begin(), outputChars3.end());

	// Check if the output is correct.
	EXPECT_TRUE(result.find(output1) != std::string::npos);
	EXPECT_TRUE(result.find(output2) != std::string::npos);
	EXPECT_TRUE(result.find(output3) != std::string::npos);
	EXPECT_EQ(HTTPStatusCodes::getCode(result), HTTPStatusCodes::getCode(HTTPStatusCodes::success("")));
}

// Checks if the program can successfully handle a check request with a hash which is not in the database.
TEST(CheckRequestTests, SingleHashNoMatch)
{
	// Set up the test.
	errno = 0;

	MockDatabase database;
	MockJDDatabase jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr);
	std::vector<MethodOut> v;

	EXPECT_CALL(database,hashToMethods("2c7f46d4f57cf9e66b03213358c7ddb5")).WillOnce(testing::Return(v));

	// Check if the output is correct.
	std::string result = handler.handleRequest("chck", "2c7f46d4f57cf9e66b03213358c7ddb5", nullptr);
	EXPECT_EQ(result, HTTPStatusCodes::success("No results found."));
}

// Checks if the program can successfully handle a check request with multiple hashes, all with one match.
TEST(CheckRequestTests, MultipleHashOneMatch)
{
	// Set up the test.
	MockDatabase database;
	MockJDDatabase jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr);
	std::vector<MethodOut> v;
	std::vector<MethodOut> v2;
	v.push_back(testMethod2);

	EXPECT_CALL(database, hashToMethods("2c7f46d4f57cf9e66b03213358c7ddb5")).WillOnce(testing::Return(v2));
	EXPECT_CALL(database, hashToMethods("06f73d7ab46184c55bf4742b9428a4c0")).WillOnce(testing::Return(v));
	EXPECT_CALL(database, hashToMethods("137fed017b6159acc0af30d2c6b403a5")).WillOnce(testing::Return(v2));

	std::vector<char> inputFunctionChars = {};
	Utility::appendBy(
		inputFunctionChars,
		{"2c7f46d4f57cf9e66b03213358c7ddb5", "06f73d7ab46184c55bf4742b9428a4c0", "137fed017b6159acc0af30d2c6b403a5"},
		ENTRY_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string inputFunction(inputFunctionChars.begin(), inputFunctionChars.end());
	std::string result = handler.handleRequest("chck", inputFunction, nullptr);

	std::vector<char> outputChars1 = {};
	Utility::appendBy(outputChars1,
					  {"2c7f46d4f57cf9e66b03213358c7ddb5", "1", "2", "1e134519e9365151e9fa7e4aadf981e115a74421", "2",
					   "1e134519e9365151e9fa7e4aadf981e115a74421", "TestMethod1", "Test1/Test2/TestFile1.cpp", "69",
					   "1", "1", "f1a028d7-3845-41df-bec1-2e16c49e4c35"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output1(outputChars1.begin(), outputChars1.end());
	std::vector<char> outputChars2 = {};
	Utility::appendBy(outputChars2,
					  {"06f73d7ab46184c55bf4742b9428a4c0", "3", "4", "1e134519e9365151e9fa7e4aadf981e115a74422", "4",
					   "1e134519e9365151e9fa7e4aadf981e115a74422", "TestMethod2", "Test3/Test4/TestFile2.cpp", "42",
					   "1", "2", "f1a028d7-3845-41df-bec1-2e16c49e4c35", "8b55fa97-5442-48f7-969c-793664388264"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output2(outputChars2.begin(), outputChars2.end());
	std::vector<char> outputChars3 = {};
	Utility::appendBy(outputChars3,
					  {"137fed017b6159acc0af30d2c6b403a5", "5", "6", "1e134519e9365151e9fa7e4aadf981e115a84422", "6",
					   "1e134519e9365151e9fa7e4aadf981e115a84422", "TestMethod3", "Test5/Test6/TestFile3.cpp", "420",
					   "1", "3", "f1a028d7-3845-41df-bec1-2e16c49e4c35", "fa37bbe0-ef68-4653-800c-d5bf30dcc7ef",
					   "5a726b9e-6173-4fc1-831b-54f98f6e760c"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output3(outputChars3.begin(), outputChars3.end());

	// Check if the output is correct.
	EXPECT_FALSE(result.find(output1) != std::string::npos);
	EXPECT_TRUE(result.find(output2) != std::string::npos);
	EXPECT_FALSE(result.find(output3) != std::string::npos);
	EXPECT_EQ(HTTPStatusCodes::getCode(result), HTTPStatusCodes::getCode(HTTPStatusCodes::success("")));
}

// Checks if the program can successfully handle a check request with one hash, having multiple matches.
TEST(CheckRequestTests, OneHashMultipleMatches)
{
	// Set up the test.
	errno = 0;

	MockDatabase database;
	MockJDDatabase jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr);
	std::vector<MethodOut> v;
	v.push_back(testMethod1);
	v.push_back(testMethod4);

	EXPECT_CALL(database, hashToMethods("2c7f46d4f57cf9e66b03213358c7ddb5")).WillOnce(testing::Return(v));
	std::string result = handler.handleRequest("chck", "2c7f46d4f57cf9e66b03213358c7ddb5", nullptr);

	std::vector<char> outputChars1 = {};
	Utility::appendBy(outputChars1,
					  {"2c7f46d4f57cf9e66b03213358c7ddb5", "1", "2", "1e134519e9365151e9fa7e4aadf981e115a74421", "2",
					   "1e134519e9365151e9fa7e4aadf981e115a74421", "TestMethod1", "Test1/Test2/TestFile1.cpp", "69",
					   "1", "1", "f1a028d7-3845-41df-bec1-2e16c49e4c35"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output1(outputChars1.begin(), outputChars1.end());
	std::vector<char> outputChars4 = {};
	Utility::appendBy(outputChars4,
					  {"2c7f46d4f57cf9e66b03213358c7ddb5", "7", "8", "1e134519e9465151e9fa7e4aadf981e115a84422", "8",
					   "1e134519e9465151e9fa7e4aadf981e115a84422", "TestMethod4", "Test7/Test8/TestFile4.cpp", "69",
					   "1", "1", "f1a028d7-3845-41df-bec1-2e16c49e4c35"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output4(outputChars4.begin(), outputChars4.end());

	// Check if the output is correct.
	EXPECT_TRUE(result.find(output1) != std::string::npos);
	EXPECT_TRUE(result.find(output4) != std::string::npos);
	EXPECT_EQ(HTTPStatusCodes::getCode(result), HTTPStatusCodes::getCode(HTTPStatusCodes::success("")));
}

// Check if the program can successfully handle a check request with multiple hashes, having multiple matches.
TEST(CheckRequestTests, MultipleHashesMultipleMatches)
{
	// Set up the test.
	errno = 0;

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

	// We expect some calls towards the database.
	EXPECT_CALL(database, hashToMethods("2c7f46d4f57cf9e66b03213358c7ddb5")).WillOnce(testing::Return(v1));
	EXPECT_CALL(database, hashToMethods("06f73d7ab46184c55bf4742b9428a4c0")).WillOnce(testing::Return(v2));
	EXPECT_CALL(database, hashToMethods("137fed017b6159acc0af30d2c6b403a5")).WillOnce(testing::Return(v3));

	std::vector<char> inputFunctionChars = {};
	Utility::appendBy(
		inputFunctionChars,
		{"2c7f46d4f57cf9e66b03213358c7ddb5", "06f73d7ab46184c55bf4742b9428a4c0", "137fed017b6159acc0af30d2c6b403a5"},
		ENTRY_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string inputFunction(inputFunctionChars.begin(), inputFunctionChars.end());
	std::string result = handler.handleRequest("chck", inputFunction, nullptr);

	std::vector<char> outputChars1 = {};
	Utility::appendBy(outputChars1,
					  {"2c7f46d4f57cf9e66b03213358c7ddb5", "1", "2", "1e134519e9365151e9fa7e4aadf981e115a74421", "2",
					   "1e134519e9365151e9fa7e4aadf981e115a74421", "TestMethod1", "Test1/Test2/TestFile1.cpp", "69",
					   "1", "1", "f1a028d7-3845-41df-bec1-2e16c49e4c35"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output1(outputChars1.begin(), outputChars1.end());
	std::vector<char> outputChars2 = {};
	Utility::appendBy(outputChars2,
					  {"06f73d7ab46184c55bf4742b9428a4c0", "3", "4", "1e134519e9365151e9fa7e4aadf981e115a74422", "4",
					   "1e134519e9365151e9fa7e4aadf981e115a74422", "TestMethod2", "Test3/Test4/TestFile2.cpp", "42",
					   "1", "2", "f1a028d7-3845-41df-bec1-2e16c49e4c35", "8b55fa97-5442-48f7-969c-793664388264"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output2(outputChars2.begin(), outputChars2.end());
	std::vector<char> outputChars3 = {};
	Utility::appendBy(outputChars3,
					  {"137fed017b6159acc0af30d2c6b403a5", "5", "6", "1e134519e9365151e9fa7e4aadf981e115a84422", "6",
					   "1e134519e9365151e9fa7e4aadf981e115a84422", "TestMethod3", "Test5/Test6/TestFile3.cpp", "420",
					   "1", "3", "f1a028d7-3845-41df-bec1-2e16c49e4c35", "fa37bbe0-ef68-4653-800c-d5bf30dcc7ef",
					   "5a726b9e-6173-4fc1-831b-54f98f6e760c"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output3(outputChars3.begin(), outputChars3.end());
	std::vector<char> outputChars4 = {};
	Utility::appendBy(outputChars4,
					  {"2c7f46d4f57cf9e66b03213358c7ddb5", "7", "8", "1e134519e9465151e9fa7e4aadf981e115a84422", "8",
					   "1e134519e9465151e9fa7e4aadf981e115a84422", "TestMethod4", "Test7/Test8/TestFile4.cpp", "69",
					   "1", "1", "f1a028d7-3845-41df-bec1-2e16c49e4c35"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output4(outputChars4.begin(), outputChars4.end());
	std::vector<char> outputChars5 = {};
	Utility::appendBy(outputChars5,
					  {"06f73d7ab46184c55bf4742b9428a4c0", "9", "10", "1e134519e9765151e9fa7e4aadf981e115a84422", "10",
					   "1e134519e9765151e9fa7e4aadf981e115a84422", "TestMethod5", "Test9/Test10/TestFile5.cpp", "42",
					   "1", "2", "f1a028d7-3845-41df-bec1-2e16c49e4c35", "8b55fa97-5442-48f7-969c-793664388264"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output5(outputChars5.begin(), outputChars5.end());

	// Check if the output is correct. 
	EXPECT_TRUE(result.find(output1) != std::string::npos);
	EXPECT_TRUE(result.find(output2) != std::string::npos);
	EXPECT_TRUE(result.find(output3) != std::string::npos);
	EXPECT_TRUE(result.find(output4) != std::string::npos);
	EXPECT_TRUE(result.find(output5) != std::string::npos);
	EXPECT_EQ(HTTPStatusCodes::getCode(result), HTTPStatusCodes::getCode(HTTPStatusCodes::success("")));
}

// Checks if the program correctly identifies an invalid hash in the input.
TEST(CheckRequestTests, InvalidHash)
{
	// Set up the test.
	errno = 0;

	RequestHandler handler;
	std::string request = "hello_I'm_an_invalid_hash";

	// Check if the output is correct.
	std::string output = handler.handleRequest("chck", request, nullptr);
	ASSERT_EQ(output, HTTPStatusCodes::clientError("Invalid hash presented."));
}

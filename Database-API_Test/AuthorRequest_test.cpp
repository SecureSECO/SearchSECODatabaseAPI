/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "DatabaseMock.cpp"
#include "HTTPStatus.h"
#include "RequestHandler.h"
#include "JDDatabaseMock.cpp"
#include "RaftConsensusMock.cpp"

#include <gtest/gtest.h>
#include <iostream>

std::string fieldDel(1, FIELD_DELIMITER_CHAR);

// Checks if two authors are equal. I.e., they have the same contents.
MATCHER_P(authorEqual, author, "")
{
	return arg.name == author.name && arg.mail == author.mail;
}

// Tests if program correctly retrieves an author with one request and one (hard-coded) match.
TEST(GetAuthorRequest, OneRequestOneMatch)
{
	// Set up the test.
	errno = 0;

	MockDatabase database;
	MockRaftConsensus raftConsensus;
	MockJDDatabase jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	std::string request = "47919e8f-7103-48a3-9514-3f2d9d49ac61";
	std::vector<char> outputChars = {};
	Utility::appendBy(outputChars, {"Author", "author@mail.com", "47919e8f-7103-48a3-9514-3f2d9d49ac61"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output(outputChars.begin(), outputChars.end());

	Author author("Author", "author@mail.com");

	EXPECT_CALL(database, idToAuthor("47919e8f-7103-48a3-9514-3f2d9d49ac61")).WillOnce(testing::Return(author));

	// Check if the output is correct.
	std::string result = handler.handleRequest("idau", request, nullptr);
	ASSERT_EQ(result, HTTPStatusCodes::success(output));
}

// Tests if program correctly retrieves an author with one request and no match.
TEST(GetAuthorRequest, OneRequestNoMatch)
{
	// Set up the test.
	errno = 0;

	MockDatabase database;
	RequestHandler handler;
	MockRaftConsensus raftConsensus;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	std::string request = "47919e8f-7103-48a3-9514-3f2d9d49ac61";
	std::string output = "No results found.";
	Author author("","");

	EXPECT_CALL(database, idToAuthor("47919e8f-7103-48a3-9514-3f2d9d49ac61")).WillOnce(testing::Return(author));

	// Check if the output is correct.
	std::string result = handler.handleRequest("idau", request, nullptr);
	ASSERT_EQ(result, HTTPStatusCodes::success(output));
}

// Tests if program correctly retrieves an author with multiple requests and one match pre request.
TEST(GetAuthorRequest, MultipleRequestMultipleMatch)
{
	// Set up the test.
	errno = 0;

	MockDatabase database;
	RequestHandler handler;
	MockRaftConsensus raftConsensus;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	std::vector<char> requestChars = {};
	Utility::appendBy(requestChars, {"47919e8f-7103-48a3-9514-3f2d9d49ac61", "41ab7373-8f24-4a03-83dc-621036d99f34"},
					  ENTRY_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string request(requestChars.begin(), requestChars.end());

	std::vector<char> outputChars1 = {};
	Utility::appendBy(outputChars1, {"Author1", "author1@mail.com", "47919e8f-7103-48a3-9514-3f2d9d49ac61"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(outputChars1, {"Author2", "author2@mail.com", "41ab7373-8f24-4a03-83dc-621036d99f34"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output1(outputChars1.begin(), outputChars1.end());
	std::vector<char> outputChars2 = {};
	Utility::appendBy(outputChars2, {"Author2", "author2@mail.com", "41ab7373-8f24-4a03-83dc-621036d99f34"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(outputChars2, {"Author1", "author1@mail.com", "47919e8f-7103-48a3-9514-3f2d9d49ac61"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output2(outputChars2.begin(), outputChars2.end());

	Author author1("Author1", "author1@mail.com");
	Author author2("Author2", "author2@mail.com");

	EXPECT_CALL(database, idToAuthor("47919e8f-7103-48a3-9514-3f2d9d49ac61")).WillOnce(testing::Return(author1));
	EXPECT_CALL(database, idToAuthor("41ab7373-8f24-4a03-83dc-621036d99f34")).WillOnce(testing::Return(author2));

	// Check if the output is correct.
	std::string result = handler.handleRequest("idau", request, nullptr);
	ASSERT_TRUE(result == HTTPStatusCodes::success(output1) || result == HTTPStatusCodes::success(output2));
}

// Tests if program correctly retrieves an author with multiple requests and one match total.
TEST(GetAuthorRequest, MultipleRequestSingleMatch)
{
	// Set up the test.
	errno = 0;

	MockDatabase database;
	RequestHandler handler;
	MockRaftConsensus raftConsensus;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	std::vector<char> requestChars = {};
	Utility::appendBy(requestChars, {"47919e8f-7103-48a3-9514-3f2d9d49ac61", "41ab7373-8f24-4a03-83dc-621036d99f34"},
					  ENTRY_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string request(requestChars.begin(), requestChars.end());
	std::vector<char> outputChars = {};
	Utility::appendBy(outputChars, {"Author1", "author1@mail.com", "47919e8f-7103-48a3-9514-3f2d9d49ac61"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output(outputChars.begin(), outputChars.end());

	Author author1("Author1", "author1@mail.com");
	Author author2("", "");

	// Test if the request is implemented correctly.
	EXPECT_CALL(database, idToAuthor("47919e8f-7103-48a3-9514-3f2d9d49ac61")).WillOnce(testing::Return(author1));
	EXPECT_CALL(database, idToAuthor("41ab7373-8f24-4a03-83dc-621036d99f34")).WillOnce(testing::Return(author2));
	std::string result = handler.handleRequest("idau", request, nullptr);
	ASSERT_EQ(result, HTTPStatusCodes::success(output));
}

// Tests if program returns an error message when an incorrect id is given.
TEST(GetAuthorRequest, IncorrectInput)
{
	// Set up the test.
	errno = 0;

	MockDatabase database;
	RequestHandler handler;
	MockRaftConsensus raftConsensus;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	std::string request = "47919e8f710348a395143f2d9d49ac61";
	std::string output = "Error parsing author id: 47919e8f710348a395143f2d9d49ac61";

	std::string result = handler.handleRequest("idau", request, nullptr);
	ASSERT_EQ(result, HTTPStatusCodes::clientError(output));
}

// Tests if program correctly retrieves a method with one author and one match.
TEST(GetMethodByAuthorTests, SingleIDRequest)
{
	// Set up the test.
	errno = 0;

	MockDatabase database;
	RequestHandler handler;
	MockRaftConsensus raftConsensus;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	MethodID method;
	method.hash = "2c7f46d4f57cf9e66b03213358c7ddb5";
	method.projectID = 42;
	method.startVersion = 69;

	std::vector<MethodID> v;
	v.push_back(method);

	std::vector<char> outputChars = {};
	Utility::appendBy(outputChars, {"41ab7373-8f24-4a03-83dc-621036d99f34", "2c7f46d4f57cf9e66b03213358c7ddb5", "42", "69"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output(outputChars.begin(), outputChars.end());

	EXPECT_CALL(database, authorToMethods("41ab7373-8f24-4a03-83dc-621036d99f34")).WillOnce(testing::Return(v));

	// Check if the output is correct.
	std::string result = handler.handleRequest("aume", "41ab7373-8f24-4a03-83dc-621036d99f34", nullptr);
	EXPECT_EQ(result, HTTPStatusCodes::success(output));
}

// Tests if program correctly retrieves a method with multiple authors and one match per author.
TEST(GetMethodByAuthorTests, MultipleIDRequest)
{
	// Set up the test.
	errno = 0;

	MockDatabase database;
	RequestHandler handler;
	MockRaftConsensus raftConsensus;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	MethodID method1;
	method1.hash = "2c7f46d4f57cf9e66b03213358c7ddb5";
	method1.projectID = 42;
	method1.startVersion = 69;

	MethodID method2;
	method2.hash = "06f73d7ab46184c55bf4742b9428a4c0";
	method2.projectID = 42;
	method2.startVersion = 420;

	std::vector<MethodID> v1;
	v1.push_back(method1);
	std::vector<MethodID> v2;
	v2.push_back(method2);

	std::vector<char> outputChars1 = {};
	Utility::appendBy(outputChars1,
					  {"47919e8f-7103-48a3-9514-3f2d9d49ac61", "2c7f46d4f57cf9e66b03213358c7ddb5", "42", "69"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(outputChars1,
					  {"41ab7373-8f24-4a03-83dc-621036d99f34", "06f73d7ab46184c55bf4742b9428a4c0", "42", "420"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output1(outputChars1.begin(), outputChars1.end());
	std::vector<char> outputChars2 = {};
	Utility::appendBy(outputChars2,
					  {"41ab7373-8f24-4a03-83dc-621036d99f34", "06f73d7ab46184c55bf4742b9428a4c0", "42", "420"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(outputChars2,
					  {"47919e8f-7103-48a3-9514-3f2d9d49ac61", "2c7f46d4f57cf9e66b03213358c7ddb5", "42", "69"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output2(outputChars2.begin(), outputChars2.end());

	EXPECT_CALL(database, authorToMethods("47919e8f-7103-48a3-9514-3f2d9d49ac61")).WillOnce(testing::Return(v1));
	EXPECT_CALL(database, authorToMethods("41ab7373-8f24-4a03-83dc-621036d99f34")).WillOnce(testing::Return(v2));

	// Check if the output is correct.
	std::vector<char> inputFunctionChars = {};
	Utility::appendBy(inputFunctionChars, {"47919e8f-7103-48a3-9514-3f2d9d49ac61", "41ab7373-8f24-4a03-83dc-621036d99f34"},
					  ENTRY_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string inputFunction(inputFunctionChars.begin(), inputFunctionChars.end());
	std::string result = handler.handleRequest("aume", inputFunction, nullptr);
	EXPECT_TRUE(result == HTTPStatusCodes::success(output1) || result == HTTPStatusCodes::success(output2));
}

// Tests if program correctly retrieves a method with one author and no match.
TEST(GetMethodByAuthorTests, SingleIDNoMatch)
{
	// Set up the test.
	errno = 0;

	MockDatabase database;
	RequestHandler handler;
	MockRaftConsensus raftConsensus;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	std::vector<MethodID> v;

	EXPECT_CALL(database, authorToMethods("47919e8f-7103-48a3-9514-3f2d9d49ac61")).WillOnce(testing::Return(v));

	// Check if the output is correct.
	std::string result = handler.handleRequest("aume", "47919e8f-7103-48a3-9514-3f2d9d49ac61", nullptr);
	EXPECT_EQ(result, HTTPStatusCodes::success("No results found."));
}

// Tests if program correctly retrieves a method with multiple authors and one match total.
TEST(GetMethodByAuthorTests, MultipleIDOneMatch)
{
	// Set up the test.
	errno = 0;

	MockDatabase database;
	RequestHandler handler;
	MockRaftConsensus raftConsensus;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	MethodID method;
	method.hash = "2c7f46d4f57cf9e66b03213358c7ddb5";
	method.projectID = 42;
	method.startVersion = 69;

	std::vector<MethodID> v;
	std::vector<MethodID> v2;
	v.push_back(method);

	EXPECT_CALL(database, authorToMethods("47919e8f-7103-48a3-9514-3f2d9d49ac61")).WillOnce(testing::Return(v2));
	EXPECT_CALL(database, authorToMethods("41ab7373-8f24-4a03-83dc-621036d99f34")).WillOnce(testing::Return(v));

	// Check if the output is correct.
	std::vector<char> inputFunctionChars = {};
	Utility::appendBy(inputFunctionChars, {"47919e8f-7103-48a3-9514-3f2d9d49ac61", "41ab7373-8f24-4a03-83dc-621036d99f34"},
					  ENTRY_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string inputFunction(inputFunctionChars.begin(), inputFunctionChars.end());
	std::string result = handler.handleRequest("aume", inputFunction, nullptr);

	std::vector<char> outputChars = {};
	Utility::appendBy(outputChars, {"41ab7373-8f24-4a03-83dc-621036d99f34", "2c7f46d4f57cf9e66b03213358c7ddb5", "42", "69"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output(outputChars.begin(), outputChars.end());
	EXPECT_EQ(result, HTTPStatusCodes::success(output));
}

// Tests if program correctly retrieves a method with one author and multiple matches.
TEST(GetMethodByAuthorTests, OneIDMultipleMatches)
{
	// Set up the test.
	errno = 0;

	MockDatabase database;
	RequestHandler handler;
	MockRaftConsensus raftConsensus;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	MethodID method1;
	method1.hash = "2c7f46d4f57cf9e66b03213358c7ddb5";
	method1.projectID = 42;
	method1.startVersion = 69;

	MethodID method2;
	method2.hash = "06f73d7ab46184c55bf4742b9428a4c0";
	method2.projectID = 42;
	method2.startVersion = 420;

	std::vector<MethodID> v;
	v.push_back(method1);
	v.push_back(method2);

	std::vector<char> outputChars1 = {};
	Utility::appendBy(outputChars1, {"47919e8f-7103-48a3-9514-3f2d9d49ac61", "2c7f46d4f57cf9e66b03213358c7ddb5", "42", "69"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(outputChars1,
					  {"47919e8f-7103-48a3-9514-3f2d9d49ac61", "06f73d7ab46184c55bf4742b9428a4c0", "42", "420"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output1(outputChars1.begin(), outputChars1.end());

	std::vector<char> outputChars2 = {};
	Utility::appendBy(outputChars2,
					  {"47919e8f-7103-48a3-9514-3f2d9d49ac61", "06f73d7ab46184c55bf4742b9428a4c0", "42", "420"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(outputChars2, {"47919e8f-7103-48a3-9514-3f2d9d49ac61", "2c7f46d4f57cf9e66b03213358c7ddb5", "42", "69"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output2(outputChars2.begin(), outputChars2.end());

	EXPECT_CALL(database, authorToMethods("47919e8f-7103-48a3-9514-3f2d9d49ac61")).WillOnce(testing::Return(v));

	// Check if the output is correct.
	std::string result = handler.handleRequest("aume", "47919e8f-7103-48a3-9514-3f2d9d49ac61", nullptr);

	EXPECT_TRUE(result == HTTPStatusCodes::success(output1) || result == HTTPStatusCodes::success(output2));
}

// Tests if program correctly retrieves a method with multiple authors and multiple matches.
TEST(GetMethodByAuthorTests, MultipleIDsMultipleMatches)
{
	// Set up the test.
	MockDatabase database;
	RequestHandler handler;
	MockRaftConsensus raftConsensus;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	MethodID method1;
	method1.hash = "2c7f46d4f57cf9e66b03213358c7ddb5";
	method1.projectID = 42;
	method1.startVersion = 69;

	MethodID method2;
	method2.hash = "06f73d7ab46184c55bf4742b9428a4c0";
	method2.projectID = 42;
	method2.startVersion = 420;

	MethodID method3;
	method3.hash = "137fed017b6159acc0af30d2c6b403a5";
	method3.projectID = 69;
	method3.startVersion = 420;

	std::vector<MethodID> v1;
	std::vector<MethodID> v2;
	v1.push_back(method1);
	v1.push_back(method2);
	v2.push_back(method3);

	// We expect the following calls towards the database (mock).
	EXPECT_CALL(database, authorToMethods("47919e8f-7103-48a3-9514-3f2d9d49ac61")).WillOnce(testing::Return(v1));
	EXPECT_CALL(database, authorToMethods("41ab7373-8f24-4a03-83dc-621036d99f34")).WillOnce(testing::Return(v2));

	std::vector<char> inputFunctionChars = {};
	Utility::appendBy(inputFunctionChars,
					  {"47919e8f-7103-48a3-9514-3f2d9d49ac61", "41ab7373-8f24-4a03-83dc-621036d99f34"},
					  ENTRY_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string inputFunction(inputFunctionChars.begin(), inputFunctionChars.end());
	std::string result = handler.handleRequest("aume", inputFunction, nullptr);

	std::vector<char> outputChars1 = {};
	Utility::appendBy(outputChars1, {"47919e8f-7103-48a3-9514-3f2d9d49ac61", "2c7f46d4f57cf9e66b03213358c7ddb5", "42", "69"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output1(outputChars1.begin(), outputChars1.end());

	std::vector<char> outputChars2 = {};
	Utility::appendBy(outputChars2, {"47919e8f-7103-48a3-9514-3f2d9d49ac61", "06f73d7ab46184c55bf4742b9428a4c0", "42", "420"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output2(outputChars2.begin(), outputChars2.end());

	std::vector<char> outputChars3 = {};
	Utility::appendBy(outputChars3, {"41ab7373-8f24-4a03-83dc-621036d99f34", "137fed017b6159acc0af30d2c6b403a5", "69", "420"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string output3(outputChars3.begin(), outputChars3.end());

	// Test if the output is correct.
	EXPECT_EQ(result.size(), HTTPStatusCodes::success(output1).size() + output2.size() + output3.size());
	EXPECT_TRUE(result.find(output1) != std::string::npos);
	EXPECT_TRUE(result.find(output2) != std::string::npos);
	EXPECT_TRUE(result.find(output3) != std::string::npos);
	ASSERT_EQ(HTTPStatusCodes::getCode(result), HTTPStatusCodes::getCode(HTTPStatusCodes::success("")));
}

// Tests if program returns an error message when an incorrect id is given.
TEST(GetMethodByAuthorTests, IncorrectInput)
{
	// Set up the test.
	MockDatabase database;
	RequestHandler handler;
	MockRaftConsensus raftConsensus;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	std::string output = "Error parsing author id: 41ab73738f244a0383dc621036d99f34";

	// Test if the output is correct.
	std::string result = handler.handleRequest("aume", "41ab73738f244a0383dc621036d99f34", nullptr);
	EXPECT_EQ(result, HTTPStatusCodes::clientError(output));
}

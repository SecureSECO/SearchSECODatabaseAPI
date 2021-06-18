/*This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)*/

#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include "HTTPStatus.h"
#include "JDDatabaseMock.cpp"
#include "Definitions.h"
#include <gtest/gtest.h>
#include <vector>

// Test input part 1:
Author owner = { .name = "Owner", .mail = "owner@mail.com" };
std::vector<Hash> hashes = {"a6aa62503e2ca3310e3a837502b80df5",
			"f3a258ba6cd26c1b7d553a493c614104",
			"59bf62494932580165af0451f76be3e9" };
ProjectIn projectT1 = { .projectID = 0, .version = 0, .versionHash = "42ea965b1f326f878bebcda51c7fb4b2", .license = "MyLicense",
			.name = "MyProject", .url = "MyUrl", .owner = owner,
			.hashes = { }, .parserVersion = 1};
MethodIn methodT1_1 = { .hash = "a6aa62503e2ca3310e3a837502b80df5",
			.methodName = "Method1",
			.fileLocation = "MyProject/Method1.cpp",
			.lineNumber = 1, .authors = { owner } };
MethodIn methodT1_2 = { .hash = "f3a258ba6cd26c1b7d553a493c614104",
			.methodName = "Method2",
			.fileLocation = "MyProject/Method2.cpp",
			.lineNumber = 11, .authors = { owner } };
MethodIn methodT1_3 = { .hash = "59bf62494932580165af0451f76be3e9",
			.methodName = "Method3",
			.fileLocation = "MyProject/Method3.cpp",
			.lineNumber = 31, .authors = { owner } };

// Test input part 2:
Author author1 = { .name = "Author 1", .mail = "author1@mail.com" };
Author author2 = { .name = "Author 2", .mail = "author2@mail.com" };
Author author3 = { .name = "Author 3", .mail = "author3@mail.com" };
ProjectIn projectT2 = { .projectID = 398798723, .version = 1618222334, .versionHash = "05a647eeb4954187fa5ac00942054cdc",
			  .license = "MyLicense", .name = "MyProject",
			  .url = "MyUrl", .owner = owner, .hashes = { }, .parserVersion = 1 };
MethodIn methodT2_1 = { .hash = "a6aa62503e2ca3310e3a837502b80df5",
			.methodName = "Method1",
			.fileLocation = "MyProject/Method1.cpp",
			.lineNumber = 1, .authors = { author1, author2, author3 } };
MethodIn methodT2_2 = { .hash = "f3a258ba6cd26c1b7d553a493c614104",
			.methodName = "Method2",
			.fileLocation = "MyProject/Method2.cpp",
			.lineNumber = 999, .authors = { author2, owner } };
MethodIn methodT2_3 = { .hash = "59bf62494932580165af0451f76be3e9",
			.methodName = "Method3",
			.fileLocation = "MyProject/Method3.cpp",
			.lineNumber = 9999999, .authors = { owner, author2, author1, author3 } };

// Checks if two projects are equal. I.e., they have the same contents.
MATCHER_P(projectEqual, project, "")
{
	return arg.projectID  == project.projectID
		&& arg.version    == project.version
		&& arg.license    == project.license
		&& arg.name       == project.name
		&& arg.url        == project.url
		&& arg.owner.name == project.owner.name
		&& arg.owner.mail == project.owner.mail
		&& arg.parserVersion == project.parserVersion;
}

// Checks if two methods are equal. I.e., they have the same contents.
MATCHER_P(methodEqual, method, "")
{
	return arg.hash         == method.hash
		&& arg.methodName   == method.methodName
		&& arg.fileLocation == method.fileLocation
		&& arg.lineNumber   == method.lineNumber;
}

// Checks if the program can successfully handle an upload request of a project with one method
// and one author.
TEST(UploadRequest, SingleMethodSingleAuthor)
{
	RequestHandler handler;
	MockJDDatabase jddatabase;
	MockDatabase database;
	handler.initialize(&database, &jddatabase, nullptr);

	std::string requestType = "upld";

	std::vector<char> requestChars = {};
	Utility::appendBy(
		requestChars,
		{"0", "0", "42ea965b1f326f878bebcda51c7fb4b2", "MyLicense", "MyProject", "MyUrl", "Owner", "owner@mail.com", "1"},
		FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	requestChars.push_back(ENTRY_DELIMITER_CHAR);
	requestChars.push_back(ENTRY_DELIMITER_CHAR);
	Utility::appendBy(
		requestChars,
		{"a6aa62503e2ca3310e3a837502b80df5", "Method1", "MyProject/Method1.cpp", "1", "1", "Owner", "owner@mail.com"},
		FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string request(requestChars.begin(), requestChars.end());

	EXPECT_CALL(database, addProject(projectEqual(projectT1))).Times(1);
	EXPECT_CALL(database, addMethod(methodEqual(methodT1_1), projectEqual(projectT1), -1, 1, true)).Times(1);

	std::string result = handler.handleRequest(requestType, request, nullptr);
	ASSERT_EQ(result, HTTPStatusCodes::success("Your project has been successfully added to the database."));
}

// Tests if the program can successfully handle an upload request of a project with multiple methods,
// each having a single author.
TEST(UploadRequest, MultipleMethodsSingleAuthor)
{
	std::string requestType = "upld";

	std::vector<char> requestChars = {};
	Utility::appendBy(
		requestChars,
		{"0", "0", "42ea965b1f326f878bebcda51c7fb4b2", "MyLicense", "MyProject", "MyUrl", "Owner", "owner@mail.com", "1"},
		FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	requestChars.push_back(ENTRY_DELIMITER_CHAR);
	requestChars.push_back(ENTRY_DELIMITER_CHAR);
	Utility::appendBy(
		requestChars,
		{"a6aa62503e2ca3310e3a837502b80df5", "Method1", "MyProject/Method1.cpp", "1", "1", "Owner", "owner@mail.com"},
		FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(
		requestChars,
		{"f3a258ba6cd26c1b7d553a493c614104", "Method2", "MyProject/Method2.cpp", "11", "1", "Owner", "owner@mail.com"},
		FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(
		requestChars,
		{"59bf62494932580165af0451f76be3e9", "Method3", "MyProject/Method3.cpp", "31", "1", "Owner", "owner@mail.com"},
		FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string request(requestChars.begin(), requestChars.end());

	RequestHandler handler;
	MockDatabase database;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, nullptr);

	EXPECT_CALL(database, addProject(projectEqual(projectT1))).Times(1);
	EXPECT_CALL(database, addMethod(methodEqual(methodT1_1), projectEqual(projectT1), -1, 1, true)).Times(1);
	EXPECT_CALL(database, addMethod(methodEqual(methodT1_2), projectEqual(projectT1), -1, 1, true)).Times(1);
	EXPECT_CALL(database, addMethod(methodEqual(methodT1_3), projectEqual(projectT1), -1, 1, true)).Times(1);

	std::string result = handler.handleRequest(requestType, request, nullptr);
	ASSERT_EQ(result, HTTPStatusCodes::success("Your project has been successfully added to the database."));
}

// Tests if program can successfully handle an upload request with multiple methods
// and multiple authors for each method.
TEST(UploadRequest, MultipleMethodsMultipleAuthors)
{
	std::string requestType = "upld";
	std::vector<char> requestChars = {};
	Utility::appendBy(requestChars,
					  {"398798723", "1618222334", "05a647eeb4954187fa5ac00942054cdc", "MyLicense", "MyProject", "MyUrl",
					   "Owner", "owner@mail.com", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	requestChars.push_back(ENTRY_DELIMITER_CHAR);
	requestChars.push_back(ENTRY_DELIMITER_CHAR);
	Utility::appendBy(requestChars,
					  {"a6aa62503e2ca3310e3a837502b80df5", "Method1", "MyProject/Method1.cpp", "1", "3", "Author 1",
					   "author1@mail.com", "Author 2", "author2@mail.com", "Author 3", "author3@mail.com"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(requestChars,
					  {"f3a258ba6cd26c1b7d553a493c614104", "Method2", "MyProject/Method2.cpp", "999", "2", "Author 2",
					   "author2@mail.com", "Owner", "owner@mail.com"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(requestChars,
					  {"59bf62494932580165af0451f76be3e9", "Method3", "MyProject/Method3.cpp", "9999999", "4", "Owner",
					   "owner@mail.com", "Author 2", "author2@mail.com", "Author 1", "author1@mail.com", "Author 3",
					   "author3@mail.com"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string request(requestChars.begin(), requestChars.end());

	RequestHandler handler;
	MockDatabase database;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, nullptr);

	EXPECT_CALL(database, addProject(projectEqual(projectT2))).Times(1);
	EXPECT_CALL(database, addMethod(methodEqual(methodT2_1), projectEqual(projectT2), -1, 1, true)).Times(1);
	EXPECT_CALL(database, addMethod(methodEqual(methodT2_2), projectEqual(projectT2), -1, 1, true)).Times(1);
	EXPECT_CALL(database, addMethod(methodEqual(methodT2_3), projectEqual(projectT2), -1, 1, true)).Times(1);

	std::string result = handler.handleRequest(requestType, request, nullptr);
	ASSERT_EQ(result, HTTPStatusCodes::success("Your project has been successfully added to the database."));
}

// Tests if the program can handle an upload request with invalid project data, too many arguments.
TEST(UploadRequest, InvalidProjectSize)
{
	std::vector<char> requestChars = {};
	Utility::appendBy(requestChars,
					  {"398798723", "1618222334", "05a647eeb4954187fa5ac00942054cdc", "MyLicense", "MyProject", "MyUrl",
					   "Owner", "owner", "stars@mail.com", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	requestChars.push_back(ENTRY_DELIMITER_CHAR);
	requestChars.push_back(ENTRY_DELIMITER_CHAR);
	std::string request(requestChars.begin(), requestChars.end());

	RequestHandler handler;
	std::string result = handler.handleRequest("upld", request, nullptr);
	ASSERT_EQ(result, HTTPStatusCodes::clientError("Error parsing project data."));
}

// Tests if the program can handle an upload request with invalid project data, non-integer id.
TEST(UploadRequest, InvalidProjectID)
{
	std::vector<char> requestChars = {};
	Utility::appendBy(requestChars,
					  {"xabs398798723", "1618222334", "05a647eeb4954187fa5ac00942054cdc", "MyLicense", "MyProject",
					   "MyUrl", "Owner", "owner@mail.com", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	requestChars.push_back(ENTRY_DELIMITER_CHAR);
	requestChars.push_back(ENTRY_DELIMITER_CHAR);
	std::string request(requestChars.begin(), requestChars.end());

	RequestHandler handler;
	std::string result = handler.handleRequest("upld", request, nullptr);
	ASSERT_EQ(result, HTTPStatusCodes::clientError("Error parsing project data."));
}


// Tests if the program can handle an upload request with invalid project data, non-integer version.
TEST(UploadRequest, InvalidProjectVersion)
{
	std::vector<char> requestChars = {};
	Utility::appendBy(requestChars,
					  {"398798723", "xabs1618222334", "05a647eeb4954187fa5ac00942054cdc", "MyLicense", "MyProject",
					   "MyUrl", "Owner", "owner@mail.com", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	requestChars.push_back(ENTRY_DELIMITER_CHAR);
	requestChars.push_back(ENTRY_DELIMITER_CHAR);
	std::string request(requestChars.begin(), requestChars.end());

	RequestHandler handler;
	std::string result = handler.handleRequest("upld", request, nullptr);
	ASSERT_EQ(result, HTTPStatusCodes::clientError("Error parsing project data."));
}

// Tests if the program can handle an upload request with invalid method data, too few arguments.
TEST(UploadRequest, InvalidMethodSizeSmall)
{
	std::vector<char> requestChars = {};
	Utility::appendBy(requestChars,
					  {"398798723", "1618222334", "05a647eeb4954187fa5ac00942054cdc", "MyLicense", "MyProject", "MyUrl",
					   "Owner", "owner@mail.com", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	requestChars.push_back(ENTRY_DELIMITER_CHAR);
	requestChars.push_back(ENTRY_DELIMITER_CHAR);
	Utility::appendBy(requestChars, {"a6aa62503e2ca3310e3a837502b80df5", "Method1", "MyProject/Method1.cpp"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string request(requestChars.begin(), requestChars.end());

	RequestHandler handler;
	std::string result = handler.handleRequest("upld", request, nullptr);
	ASSERT_EQ(result, HTTPStatusCodes::clientError("Error parsing method 1."));
}

// Tests if the program can handle an upload request with invalid method data, too many arguments.
TEST(UploadRequest, InvalidMethodSizeLarge)
{
	std::vector<char> requestChars = {};
	Utility::appendBy(requestChars,
					  {"398798723", "1618222334", "05a647eeb4954187fa5ac00942054cdc", "MyLicense", "MyProject", "MyUrl",
					   "Owner", "owner@mail.com", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	requestChars.push_back(ENTRY_DELIMITER_CHAR);
	requestChars.push_back(ENTRY_DELIMITER_CHAR);
	Utility::appendBy(requestChars,
					  {"a6aa62503e2ca3310e3a837502b80df5", "Method1", "MyProject/Method1.cpp", "1", "2", "Author 1",
					   "author1@mail.com", "Author 2", "author2@mail.com", "Author 3", "author3@mail.com"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string request(requestChars.begin(), requestChars.end());

	RequestHandler handler;
	std::string result = handler.handleRequest("upld", request, nullptr);
	ASSERT_EQ(result, HTTPStatusCodes::clientError("Error parsing method 1."));
}

// Tests if the program can handle an upload request with invalid method data, invalid method hash.
TEST(UploadRequest, InvalidMethodHash)
{
	std::vector<char> requestChars = {};
	Utility::appendBy(requestChars,
					  {"398798723", "1618222334", "05a647eeb4954187fa5ac00942054cdc", "MyLicense", "MyProject", "MyUrl",
					   "Owner", "owner@mail.com", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	requestChars.push_back(ENTRY_DELIMITER_CHAR);
	requestChars.push_back(ENTRY_DELIMITER_CHAR);
	Utility::appendBy(requestChars,
					  {"a6aa62503e2ca3310e3a837502b80df5", "Method1", "MyProject/Method1.cpp", "1", "3", "Author 1",
					   "author1@mail.com", "Author 2", "author2@mail.com", "Author 3", "author3@mail.com"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	Utility::appendBy(requestChars,
					  {"a6aa62503e2ca3310e3a837502b80df5xx", "Method1", "MyProject/Method1.cpp", "1", "3", "Author 1",
					   "author1@mail.com", "Author 2", "author2@mail.com", "Author 3", "author3@mail.com"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string request(requestChars.begin(), requestChars.end());

	RequestHandler handler;
	std::string result = handler.handleRequest("upld", request, nullptr);
	ASSERT_EQ(result, HTTPStatusCodes::clientError("Error parsing method 2."));
}

// Tests if the program can handle an upload request with invalid method data, non-integer line number.
TEST(UploadRequest, InvalidMethodLine)
{
	std::vector<char> requestChars = {};
	Utility::appendBy(requestChars,
					  {"398798723", "1618222334", "05a647eeb4954187fa5ac00942054cdc", "MyLicense", "MyProject", "MyUrl",
					   "Owner", "owner@mail.com", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	requestChars.push_back(ENTRY_DELIMITER_CHAR);
	requestChars.push_back(ENTRY_DELIMITER_CHAR);
	Utility::appendBy(requestChars,
					  {"a6aa62503e2ca3310e3a837502b80df5", "Method1", "MyProject/Method1.cpp", "not_an_integer", "3",
					   "Author 1", "author1@mail.com", "Author 2", "author2@mail.com", "Author 3", "author3@mail.com"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string request(requestChars.begin(), requestChars.end());

	RequestHandler handler;
	std::string result = handler.handleRequest("upld", request, nullptr);
	ASSERT_EQ(result, HTTPStatusCodes::clientError("Error parsing method 1."));
}


// Tests if the program can handle an upload request with invalid method data, non-integer number of authors.
TEST(UploadRequest, InvalidMethodAuthorLines)
{
	std::vector<char> requestChars = {};
	Utility::appendBy(requestChars,
					  {"398798723", "1618222334", "05a647eeb4954187fa5ac00942054cdc", "MyLicense", "MyProject", "MyUrl",
					   "Owner", "owner@mail.com", "1"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	requestChars.push_back(ENTRY_DELIMITER_CHAR);
	requestChars.push_back(ENTRY_DELIMITER_CHAR);
	Utility::appendBy(requestChars,
					  {"a6aa62503e2ca3310e3a837502b80df5", "Method1", "MyProject/Method1.cpp", "1", "not_an_integer",
					   "Author 1", "author1@mail.com", "Author 2", "author2@mail.com", "Author 3", "author3@mail.com"},
					  FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	std::string request(requestChars.begin(), requestChars.end());

	RequestHandler handler;
	std::string result = handler.handleRequest("upld", request, nullptr);
	ASSERT_EQ(result, HTTPStatusCodes::clientError("Error parsing method 1."));
}

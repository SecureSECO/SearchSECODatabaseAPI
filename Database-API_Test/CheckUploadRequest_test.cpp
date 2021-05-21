/*This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)*/

#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include "JDDatabaseMock.cpp"
#include <gtest/gtest.h>

// Checks if two projects are equal. I.e., they have the same contents.
MATCHER_P(projectEqual, project, "")
{
	return arg.projectID  == project.projectID
	    && arg.version    == project.version
	    && arg.license    == project.license
	    && arg.name       == project.name
	    && arg.url        == project.url
	    && arg.owner.name == project.owner.name
	    && arg.owner.mail == project.owner.mail;

}

// Checks if two methods are equal. I.e., they have the same contents.
MATCHER_P(methodEqual, method, "")
{
	return arg.hash         == method.hash
		&& arg.methodName   == method.methodName
		&& arg.fileLocation == method.fileLocation
		&& arg.lineNumber   == method.lineNumber;
}

// Tests if program works correctly with one request and one (hard-coded) match.
TEST(CheckUploadRequest, OneRequestOneMatch)
{
	MockDatabase database;
	MockJDDatabase jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr);

	std::string request = "0?0?MyLicense?MyProject?MyUrl?Owner?owner@mail.com\n"
						  "a6aa62503e2ca3310e3a837502b80df5?Method1?"
						  "MyProject/Method1.cpp?1?1?Owner?owner@mail.com";
	std::string output = "a6aa62503e2ca3310e3a837502b80df5?0?0?Method1?"
						 "MyProject/Method1.cpp?1?1?f1a028d7-3845-41df-bec1-2e16c49e4c35\n";
	std::string authorID = "f1a028d7-3845-41df-bec1-2e16c49e4c35";
	MethodOut method1out = { .hash = "a6aa62503e2ca3310e3a837502b80df5", .projectID = 0,
							 .version = 0, .methodName = "Method1",
							 .fileLocation = "MyProject/Method1.cpp", .lineNumber = 1,
							 .authorIDs = { "f1a028d7-3845-41df-bec1-2e16c49e4c35" } };
	Author author = { .name = "Owner", .mail = "owner@mail.com" };
	ProjectIn project = { .projectID = 0, .version = 0, .license = "MyLicense",
						.name = "MyProject", .url = "MyUrl", .owner = author,
						.hashes = { } };
	MethodIn method1in = { .hash = "a6aa62503e2ca3310e3a837502b80df5",
						   .methodName = "Method1", .fileLocation = "MyProject/Method1.cpp",
						   .lineNumber = 1, .authors = { author } };
	std::vector<MethodOut> v;
	v.push_back(method1out);

	EXPECT_CALL(database, addProject(projectEqual(project))).Times(1);
	EXPECT_CALL(database, addMethod(methodEqual(method1in), projectEqual(project))).Times(1);
	EXPECT_CALL(database, hashToMethods("a6aa62503e2ca3310e3a837502b80df5")).WillOnce(testing::Return(v));
	std::string result = handler.handleRequest("chup", request, nullptr);
	ASSERT_EQ(result, output);
}

// Tests if program correctly handles a checkupload which cannot be converted to hashes.
TEST(CheckUploadRequest, HashConversionError)
{
	RequestHandler handler;

	std::string request = "0?0?MyLicense?MyProject?MyUrl?Owner?owner@mail.com\n"
                                                  "a6aa62503e2ca3310e3a837502b80df5xx?Method1?"
                                                  "MyProject/Method1.cpp?1?1?Owner?owner@mail.com";

	std::string result = handler.handleRequest("chup", request, nullptr);
	ASSERT_EQ(result, "Error parsing hashes.");
}

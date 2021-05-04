/*This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
 Copyright Utrecht University(Department of Informationand Computing Sciences)*/

#include "RequestHandler.h"
#include "DatabaseHandler.h"
#include <gtest/gtest.h>

// Tests if program works correctly with one request and one (hard-coded) match.
TEST(CheckUploadRequest, OneRequestOneMatch)
{
	DatabaseHandler database;
	RequestHandler handler;
	handler.initialize(&database, "127.0.0.1", 9042);

	std::string request = "0?0?MyLicense?MyProject?MyUrl?Owner?owner@mail.com?0\n"
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
	Project project = { .projectID = 0, .version = 0, .license = "MyLicense",
						.name = "MyProject", .url = "MyUrl", .owner = author,
						.stars = 0, .hashes = { } };
	MethodIn method1in = { .hash = "a6aa62503e2ca3310e3a837502b80df5",
						   .methodName = "Method1", .fileLocation = "MyProject/Method1.cpp",
						   .lineNumber = 1, .authors = { author } };
	std::vector<MethodOut> v;
	v.push_back(method1out);
	handler.handleRequest("upld", request);
	std::string result = handler.handleRequest("chck", "a6aa62503e2ca3310e3a837502b80df5");
	ASSERT_EQ(result[0], 'a');
}

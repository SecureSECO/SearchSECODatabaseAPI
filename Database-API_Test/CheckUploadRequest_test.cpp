/*This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
 Copyright Utrecht University(Department of Informationand Computing Sciences)*/

#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include <gtest/gtest.h>

MATCHER_P(projectEqual, project, "")
{
	return arg.projectID  == project.projectID
	    && arg.version    == project.version
	    && arg.license    == project.license
	    && arg.name       == project.name
	    && arg.url        == project.url
	    && arg.owner.name == project.owner.name
	    && arg.owner.mail == project.owner.mail;
	    /*&& arg.stars      == project.stars
	    && arg.hashes     == project.hashes;*/
}

MATCHER_P(methodEqual, method, "")
{
	/*if (arg.authors.size() != method.authors.size())
	{
		return false;
	}
	else
	{
		for (int i = 0; i < method.authors.size(); i++)
		{
			if (arg.authors[i].name != method.authors[i].name || arg.authors[i].mail != method.authors[i].mail)
			{
				return false;
			}
		}*/
		return arg.hash         == method.hash
            	    && arg.methodName   == method.methodName
                    && arg.fileLocation == method.fileLocation
                    && arg.lineNumber   == method.lineNumber;
	//}
}

TEST(CheckUploadRequest, OneRequestOneMatch)
{
	MockDatabase database;
	RequestHandler handler;
	handler.initialize(&database);

	std::string request = "0?0?MyLicense?MyProject?MyUrl?Owner?owner@mail.com?0\na6aa62503e2ca3310e3a837502b80df5?Method1?MyProject/Method1.cpp?1?1?Owner?owner@mail.com";
	std::string output = "a6aa62503e2ca3310e3a837502b80df5?0?0?Method1?MyProject/Method1.cpp?1?1?f1a028d7-3845-41df-bec1-2e16c49e4c35\n";
	std::string authorID = "f1a028d7-3845-41df-bec1-2e16c49e4c35";
	MethodOut method1out = { .hash = "a6aa62503e2ca3310e3a837502b80df5", .projectID = 0, .version = 0, .methodName = "Method1", .fileLocation = "MyProject/Method1.cpp", .lineNumber = 1, .authorIDs = { "f1a028d7-3845-41df-bec1-2e16c49e4c35" }};
	Author author = { .name = "Owner", .mail = "owner@mail.com" };
	Project project = { .projectID = 0, .version = 0, .license = "MyLicense", .name = "MyProject", .url = "MyUrl", .owner = author, .stars = 0, .hashes = {} };
	MethodIn method1in = { .hash = "a6aa62503e2ca3310e3a837502b80df5", .methodName = "Method1", .fileLocation = "MyProject/Method1.cpp" , .lineNumber = 1, .authors = {author} };
	std::vector<MethodOut> v;
	v.push_back(method1out);

	EXPECT_CALL(database, addProject(projectEqual(project))).Times(1);
	EXPECT_CALL(database, addMethod(methodEqual(method1in), projectEqual(project))).Times(1);
	EXPECT_CALL(database, hashToMethods("a6aa62503e2ca3310e3a837502b80df5")).WillOnce(testing::Return(v));
	std::string result = handler.handleRequest("chup", request);
	ASSERT_EQ(result, output);
}

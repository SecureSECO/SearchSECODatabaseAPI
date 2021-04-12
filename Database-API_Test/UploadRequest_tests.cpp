#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include <gtest/gtest.h>
#include <vector>

using namespace std;

Author author = { .name = "Owner", .mail = "owner@mail.com" };
Project project = { .projectID = 0, .version = 0, .license = "MyLicense", .name = "MyProject", .url = "MyUrl", .owner = author, .stars = 0, .hashes = {} };
MethodIn method1 = { .hash = "a6aa62503e2ca3310e3a837502b80df5", .methodName = "Method1", .fileLocation = "MyProject/Method1.cpp" , .lineNumber = 1, .authors = {author} };
MethodIn method2 = { .hash = "f3a258ba6cd26c1b7d553a493c614104", .methodName = "Method2", .fileLocation = "MyProject/Method2.cpp" , .lineNumber = 11, .authors = {author} };
MethodIn method3 = { .hash = "59bf62494932580165af0451f76be3e9", .methodName = "Method3", .fileLocation = "MyProject/Method3.cpp" , .lineNumber = 31, .authors = {author} };

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

TEST(UploadRequest, OneMethodSuccess)
{
	RequestHandler handler;
	MockDatabase database;
	handler.initialize(&database);

	std::string requestType = "upld";
	std::string request = "0?0?MyLicense?MyProject?MyUrl?Owner?owner@mail.com?0\na6aa62503e2ca3310e3a837502b80df5?Method1?MyProject/Method1.cpp?1?1?Owner?owner@mail.com";

	EXPECT_CALL(database, addProject(projectEqual(project))).Times(1);
	EXPECT_CALL(database, addMethod(methodEqual(method1), projectEqual(project))).Times(1);

	string result = handler.handleRequest(requestType, request);
        ASSERT_EQ(result, "Your project is successfully added to the database.");
}

TEST(UploadRequest, MultipleMethodsSingleAuthor)
{
	std::string requestType = "upld";
	std::string request = "0?0?MyLicense?MyProject?MyUrl?Owner?owner@mail.com?0\na6aa62503e2ca3310e3a837502b80df5?Method1?MyProject/Method1.cpp?1?1?Owner?owner@mail.com\nf3a258ba6cd26c1b7d553a493c614104?Method2?MyProject/Method2.cpp?11?1?Owner?owner@mail.com\n59bf62494932580165af0451f76be3e9?Method3?MyProject/Method3.cpp?31?1?Owner?owner@mail.com";

	RequestHandler handler;
	MockDatabase database;
	handler.initialize(&database);

	//EXPECT_CALL(database, AddProject(FieldsAre(0, 0, Eq("MyLicense"), Eq("MyProject"), "MyUrl", FieldsAre("Owner", "owner@mail.com"), 0, {"a6aa62503e2ca3310e3a837502b80df5", "f3a258ba6cd26c1b7d553a493c614104"})))
	EXPECT_CALL(database, addProject(projectEqual(project)))
		.Times(1);
	EXPECT_CALL(database, addMethod(methodEqual(method1), projectEqual(project))).Times(1);
	//EXPECT_CALL(database, addMethod(ownersEqual(method1.authors), testing::_))
	//	.Times(1);
	EXPECT_CALL(database, addMethod(methodEqual(method2), projectEqual(project))).Times(1);
	//EXPECT_CALL(database, addMethod(ownersEqual(method2.authors), testing::_))
	//	.Times(1);
	EXPECT_CALL(database, addMethod(methodEqual(method3), projectEqual(project))).Times(1);
	//EXPECT_CALL(database, addMethod(ownersEqual(method3.authors), testing::_))
	//	.Times(1);

	string result = handler.handleRequest(requestType, request);
	ASSERT_EQ(result, "Your project is successfully added to the database.");
}

#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include <gtest/gtest.h>
#include <vector>

using namespace std;

/*TEST(UploadRequest, EmptyRequest)
{
    RequestHandler handler;
    EXPECT_EQ(handler.handleUploadRequest("upld", ""), );
}*/

MATCHER_P(projectEqual, project, "")
{
	return arg.projectID  == project.projectID
	    && arg.version    == project.version
	    && arg.license    == project.license
	    && arg.name       == project.name
	    && arg.url        == project.url
	    && arg.owner.name == project.owner.name
	    && arg.owner.mail == project.owner.mail
	    && arg.stars      == project.stars
	    && arg.hashes     == project.hashes;
}

MATCHER_P(methodEqual, method, "")
{
	return arg.hash         == method.hash
	    && arg.methodName   == method.methodName
	    && arg.fileLocation == method.fileLocation
	    && arg.lineNumber   == method.lineNumber;
}

MATCHER_P(ownersEqual, owners, "")
{
	if (arg.authors.size() != owners.size())
	{
		return false;
	}
	else
	{
		for (int i = 0; i < owners.size(); i++)
		{
			if (arg.authors[i].name != owners[i].name || arg.authors[i].mail != owners[i].mail)
			{
				return false;
			}
		}
		return true;
	}
}

TEST(UploadRequest, MultipleMethodsSingleAuthor)
{
	std::string requestType = "upld";
	std::string request = "0?0?MyLicense?MyProject?MyUrl?Owner?owner@mail.com?0\na6aa62503e2ca3310e3a837502b80df5?Method1?MyProject/Method1.cpp?1?1?Owner?owner@mail.com\nf3a258ba6cd26c1b7d553a493c614104?Method2?MyProject/Method2.cpp?11?1?Owner?owner@mail.com\n59bf62494932580165af0451f76be3e9?Method3?MyProject/Method3.cpp?31?1?Owner?owner@mail.com";

	RequestHandler handler;
	MockDatabase database;

	Project project;
	project.projectID = 0;
	project.version = 0;
	project.license = "MyLicense";
	project.name = "MyProject";
	project.url = "MyUrl";
	project.owner.name = "Owner";
 	project.owner.mail = "owner@mail.com";
	project.stars = 0;
	project.hashes = {};//{"a6aa62503e2ca3310e3a837502b80df5", "f3a258ba6cd26c1b7d553a493c614104"};

	Author author;
	author.name = "Owner";
	author.mail = "owner@mail.com";

	MethodIn method1;
	method1.hash = "a6aa62503e2ca3310e3a837502b80df5";
	method1.methodName = "Method1";
	method1.fileLocation = "MyProject/Method1.cpp";
	method1.lineNumber = 1;
	method1.authors = {author};

	MethodIn method2;
	method2.hash = "f3a258ba6cd26c1b7d553a493c614104";
	method2.methodName = "Method2";
	method2.fileLocation = "MyProject/Method2.cpp";
	method2.lineNumber = 11;
	method2.authors = {author};

	MethodIn method3;
	method3.hash = "59bf62494932580165af0451f76be3e9";
	method3.methodName = "Method3";
	method3.fileLocation = "MyProject/Method3.cpp";
	method3.lineNumber = 31;
	method3.authors = {author};

	//EXPECT_CALL(database, AddProject(FieldsAre(0, 0, Eq("MyLicense"), Eq("MyProject"), "MyUrl", FieldsAre("Owner", "owner@mail.com"), 0, {"a6aa62503e2ca3310e3a837502b80df5", "f3a258ba6cd26c1b7d553a493c614104"})))
	EXPECT_CALL(database, addProject(projectEqual(project)))
		.Times(1);
	EXPECT_CALL(database, addMethod(methodEqual(method1), projectEqual(project)));
	EXPECT_CALL(database, addMethod(ownersEqual(method1.authors), testing::_))
		.Times(1);
	EXPECT_CALL(database, addMethod(methodEqual(method2), projectEqual(project)));
	EXPECT_CALL(database, addMethod(ownersEqual(method2.authors), testing::_))
		.Times(1);
	EXPECT_CALL(database, addMethod(methodEqual(method3), projectEqual(project)));
	EXPECT_CALL(database, addMethod(ownersEqual(method3.authors), testing::_))
		.Times(1);

	string result = handler.handleRequest(requestType, request);
	ASSERT_EQ(result, "Your project is successfully added to the database.");
}

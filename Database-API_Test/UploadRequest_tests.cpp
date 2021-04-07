#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include <gtest/gtest.h>

/*TEST(UploadRequest, EmptyRequest)
{
    RequestHandler handler;
    EXPECT_EQ(handler.handleUploadRequest("upld"), );
}*/

TEST(UploadRequest, MultipleMethodsSingleAuthor)
{
	string request = "upld 0?0?MyLicense?MyProject?MyUrl?Owner?owner@mail.com?0\na6aa62503e2ca3310e3a837502b80df5?Method1?MyProject/Method1.cpp?1?1?Owner?owner@mail.com\nf3a258ba6cd26c1b7d553a493c614104?Method2?MyProject/Method2.cpp?11?1?Owner?owner@mail.com\n59bf62494932580165af0451f76be3e9?Method3?MyProject/Method3.cpp?31?1?Owner?owner@mail.com";

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
	project.hashes = {"a6aa62503e2ca3310e3a837502b80df5", "f3a258ba6cd26c1b7d553a493c614104"};

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

	EXPECT_CALL(database, AddProject(project))
		.Times(1);
	EXPECT_CALL(database, AddMethod(method1, project))
		.Times(1);
	EXPECT_CALL(database, AddMethod(method2, project))
		.Times(1);
	EXPECT_CALL(database, AddMethod(method3, project))
		.Times(1);

	handler.handleRequest(request)_StrEq("Your project is successfully added to the database.");
}

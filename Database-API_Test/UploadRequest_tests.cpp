#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include <gtest/gtest.h>
#include <vector>

using namespace std;

// Test input 1:
Author owner = { .name = "Owner", .mail = "owner@mail.com" };
vector<Hash> hashes = { "a6aa62503e2ca3310e3a837502b80df5", "f3a258ba6cd26c1b7d553a493c614104", "59bf62494932580165af0451f76be3e9" };
Project projectT1 = { .projectID = 0, .version = 0, .license = "MyLicense", .name = "MyProject", .url = "MyUrl", .owner = owner, .stars = 0, .hashes = { } };
MethodIn methodT1_1 = { .hash = "a6aa62503e2ca3310e3a837502b80df5", .methodName = "Method1", .fileLocation = "MyProject/Method1.cpp" , .lineNumber = 1, .authors = { owner } };
MethodIn methodT1_2 = { .hash = "f3a258ba6cd26c1b7d553a493c614104", .methodName = "Method2", .fileLocation = "MyProject/Method2.cpp" , .lineNumber = 11, .authors = { owner } };
MethodIn methodT1_3 = { .hash = "59bf62494932580165af0451f76be3e9", .methodName = "Method3", .fileLocation = "MyProject/Method3.cpp" , .lineNumber = 31, .authors = { owner } };

// Test input 2:
Author author1 = { .name = "Author 1", .mail = "author1@mail.com" };
Author author2 = { .name = "Author 2", .mail = "author2@mail.com" };
Author author3 = { .name = "Author 3", .mail = "author3@mail.com" };
Project projectT2 = { .projectID = 398798723, .version = 1618222334, .license = "MyLicense", .name = "MyProject", .url = "MyUrl", .owner = owner, .stars = 0, .hashes = { } };
MethodIn methodT2_1 = { .hash = "a6aa62503e2ca3310e3a837502b80df5", .methodName = "Method1", .fileLocation = "MyProject/Method1.cpp" , .lineNumber = 1, .authors = { author1, author2, author3 } };
MethodIn methodT2_2 = { .hash = "f3a258ba6cd26c1b7d553a493c614104", .methodName = "Method2", .fileLocation = "MyProject/Method2.cpp" , .lineNumber = 999, .authors = { author2, owner } };
MethodIn methodT2_3 = { .hash = "59bf62494932580165af0451f76be3e9", .methodName = "Method3", .fileLocation = "MyProject/Method3.cpp" , .lineNumber = 9999999, .authors = { owner, author2, author1, author3 } };

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

	string requestType = "upld";
	string request = "0?0?MyLicense?MyProject?MyUrl?Owner?owner@mail.com?0\na6aa62503e2ca3310e3a837502b80df5?Method1?MyProject/Method1.cpp?1?1?Owner?owner@mail.com";

	EXPECT_CALL(database, addProject(projectEqual(projectT1))).Times(1);
	EXPECT_CALL(database, addMethod(methodEqual(methodT1_1), projectEqual(projectT1))).Times(1);

	string result = handler.handleRequest(requestType, request);
        ASSERT_EQ(result, "Your project is successfully added to the database.");
}

TEST(UploadRequest, MultipleMethodsSingleAuthor)
{
	string requestType = "upld";
	string request = "0?0?MyLicense?MyProject?MyUrl?Owner?owner@mail.com?0\na6aa62503e2ca3310e3a837502b80df5?Method1?MyProject/Method1.cpp?1?1?Owner?owner@mail.com\nf3a258ba6cd26c1b7d553a493c614104?Method2?MyProject/Method2.cpp?11?1?Owner?owner@mail.com\n59bf62494932580165af0451f76be3e9?Method3?MyProject/Method3.cpp?31?1?Owner?owner@mail.com";

	RequestHandler handler;
	MockDatabase database;
	handler.initialize(&database);

	//EXPECT_CALL(database, AddProject(FieldsAre(0, 0, Eq("MyLicense"), Eq("MyProject"), "MyUrl", FieldsAre("Owner", "owner@mail.com"), 0, {"a6aa62503e2ca3310e3a837502b80df5", "f3a258ba6cd26c1b7d553a493c614104"})))
	EXPECT_CALL(database, addProject(projectEqual(projectT1)))
		.Times(1);
	EXPECT_CALL(database, addMethod(methodEqual(methodT1_1), projectEqual(projectT1))).Times(1);
	//EXPECT_CALL(database, addMethod(ownersEqual(method1.authors), testing::_))
	//	.Times(1);
	EXPECT_CALL(database, addMethod(methodEqual(methodT1_2), projectEqual(projectT1))).Times(1);
	//EXPECT_CALL(database, addMethod(ownersEqual(method2.authors), testing::_))
	//	.Times(1);
	EXPECT_CALL(database, addMethod(methodEqual(methodT1_3), projectEqual(projectT1))).Times(1);
	//EXPECT_CALL(database, addMethod(ownersEqual(method3.authors), testing::_))
	//	.Times(1);

	string result = handler.handleRequest(requestType, request);
	ASSERT_EQ(result, "Your project is successfully added to the database.");
}

// Do not remove the following test. (Good when actual error handling is introduced.)
/*TEST(UploadRequest, EmptyRequest)
{
	string requestType = "upld";
	string request = "";

	ASSERT_EQ(result, "Input is of the wrong format.");
}

TEST(UploadRequest, WrongTypeArguments)
{
	string requestType = "upld";
	string request = "-?-??????-\n???-?-??";

	string result = handler.handleRequest(requestType, request);
	ASSERT_EQ(result, "Input is of the wrong format.");
}*/

TEST(UploadRequest, MultipleMethodMultipleAuthors)
{
	string requestType = "upld";
	string request = "398798723?1618222334?MyLicense?MyProject?MyUrl?Owner?owner@mail.com?0\na6aa62503e2ca3310e3a837502b80df5?Method1?MyProject/Method1.cpp?1?3?Author 1?author1@mail.com?Author 2?author2@mail.com?Author 3?author3@mail.com\nf3a258ba6cd26c1b7d553a493c614104?Method2?MyProject/Method2.cpp?999?2?Author 2?author2@mail.com?Owner?owner@mail.com\n59bf62494932580165af0451f76be3e9?Method3?MyProject/Method3.cpp?9999999?3?Owner?owner@mail.com?Author 2?author2@mail.com?Author 1?author1@mail.com?Author 3?author3@mail.com";

	RequestHandler handler;
	MockDatabase database;
	handler.initialize(&database);

	EXPECT_CALL(database, addProject(projectEqual(projectT2))).Times(1);
	EXPECT_CALL(database, addMethod(methodEqual(methodT2_1), projectEqual(projectT2))).Times(1);
	EXPECT_CALL(database, addMethod(methodEqual(methodT2_2), projectEqual(projectT2))).Times(1);
	EXPECT_CALL(database, addMethod(methodEqual(methodT2_3), projectEqual(projectT2))).Times(1);

	string result = handler.handleRequest(requestType, request);
	ASSERT_EQ(result, "Your project is successfully added to the database.");
}

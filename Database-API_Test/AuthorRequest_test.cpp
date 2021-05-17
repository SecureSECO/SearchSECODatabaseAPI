/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
� Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "DatabaseMock.cpp"
#include "RequestHandler.h"
#include "JDDatabaseMock.cpp"
#include <gtest/gtest.h>
#include <iostream>

// Checks if two authors are equal. I.e., they have the same contents.
MATCHER_P(authorEqual, author, "")
{
	return arg.name == author.name && arg.mail == author.mail;
}

// Tests if program correctly retreieves an author id with one request and one (hard-coded) match.
TEST(GetAuthorIdRequest, OneRequestOneMatch)
{
	MockDatabase database;
	RequestHandler handler;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, nullptr);

	std::string request = "Author?author@mail.com\n";
	std::string output = "Author?author@mail.com?47919e8f-7103-48a3-9514-3f2d9d49ac61\n";
	Author author;
	author.name = "Author";
	author.mail = "author@mail.com";

	EXPECT_CALL(database, authorToId(authorEqual(author))).WillOnce(testing::Return("47919e8f-7103-48a3-9514-3f2d9d49ac61"));
	std::string result = handler.handleRequest("auid", request, nullptr);
	ASSERT_EQ(result, output);
}

// Tests if program correctly retreieves an author id with one request and no match.
TEST(GetAuthorIdRequest, OneRequestNoMatch)
{
	MockDatabase database;
	RequestHandler handler;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, nullptr);

	std::string request = "Author?author@mail.com\n";
	std::string output = "No results found.";
	Author author;
	author.name = "Author";
	author.mail = "author@mail.com";

	EXPECT_CALL(database, authorToId(authorEqual(author)))
		.WillOnce(testing::Return(""));
	std::string result = handler.handleRequest("auid", request, nullptr);
	ASSERT_EQ(result, output);
}

// Tests if program correctly retreieves an author id with multiple requests and one match per request.
TEST(GetAuthorIdRequest, MultipleRequestMultipleMatch)
{
	MockDatabase database;
	RequestHandler handler;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, nullptr);

	std::string request = "Author1?author1@mail.com\nAuthor2?author2@mail.com\n";
	std::string output1 = "Author1?author1@mail.com?47919e8f-7103-48a3-9514-3f2d9d49ac61\nAuthor2?author2@mail.com?"
						  "41ab7373-8f24-4a03-83dc-621036d99f34\n";
	std::string output2 = "Author2?author2@mail.com?41ab7373-8f24-4a03-83dc-621036d99f34\nAuthor1?author1@mail.com?"
						  "47919e8f-7103-48a3-9514-3f2d9d49ac61\n";
	Author author1;
	author1.name = "Author1";
	author1.mail = "author1@mail.com";
	Author author2;
	author2.name = "Author2";
	author2.mail = "author2@mail.com";

	EXPECT_CALL(database, authorToId(authorEqual(author1)))
		.WillOnce(testing::Return("47919e8f-7103-48a3-9514-3f2d9d49ac61"));
	EXPECT_CALL(database, authorToId(authorEqual(author2)))
		.WillOnce(testing::Return("41ab7373-8f24-4a03-83dc-621036d99f34"));
	std::string result = handler.handleRequest("auid", request, nullptr);
	ASSERT_TRUE(result == output1 || result == output2);
}

// Tests if program correctly retreieves an author id with multiple requests and one match total.
TEST(GetAuthorIdRequest, MultipleRequestOneMatch)
{
	MockDatabase database;
	RequestHandler handler;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, nullptr);

	std::string request = "Author1?author1@mail.com\nAuthor2?author2@mail.com\n";
	std::string output = "Author1?author1@mail.com?47919e8f-7103-48a3-9514-3f2d9d49ac61\n";
	Author author1;
	author1.name = "Author1";
	author1.mail = "author1@mail.com";
	Author author2;
	author2.name = "Author2";
	author2.mail = "author2@mail.com";

	EXPECT_CALL(database, authorToId(authorEqual(author1)))
		.WillOnce(testing::Return("47919e8f-7103-48a3-9514-3f2d9d49ac61"));
	EXPECT_CALL(database, authorToId(authorEqual(author2))).WillOnce(testing::Return(""));
	std::string result = handler.handleRequest("auid", request, nullptr);
	ASSERT_EQ(result, output);
}

// Tests if program correctly retreieves an author with one request and one (hard-coded) match.
TEST(GetAuthorRequest, OneRequestOneMatch)
{
	MockDatabase database;
	RequestHandler handler;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, nullptr);

	std::string request = "47919e8f-7103-48a3-9514-3f2d9d49ac61\n";
	std::string output = "Author?author@mail.com?47919e8f-7103-48a3-9514-3f2d9d49ac61\n";
	Author author;
	author.name = "Author";
	author.mail = "author@mail.com";

	EXPECT_CALL(database, idToAuthor("47919e8f-7103-48a3-9514-3f2d9d49ac61"))
		.WillOnce(testing::Return(author));
	std::string result = handler.handleRequest("idau", request, nullptr);
	ASSERT_EQ(result, output);
}

// Tests if program correctly retreieves an author with one request and no match.
TEST(GetAuthorRequest, OneRequestNoMatch)
{
	MockDatabase database;
	RequestHandler handler;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, nullptr);

	std::string request = "47919e8f-7103-48a3-9514-3f2d9d49ac61\n";
	std::string output = "No results found.";
	Author author;

	EXPECT_CALL(database, idToAuthor("47919e8f-7103-48a3-9514-3f2d9d49ac61")).WillOnce(testing::Return(author));
	std::string result = handler.handleRequest("idau", request, nullptr);
	ASSERT_EQ(result, output);
}

// Tests if program correctly retreieves an author with multiple requests and one match pre request.
TEST(GetAuthorRequest, MultipleRequestMultipleMatch)
{
	MockDatabase database;
	RequestHandler handler;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, nullptr);

	std::string request = "47919e8f-7103-48a3-9514-3f2d9d49ac61\n41ab7373-8f24-4a03-83dc-621036d99f34\n";
	std::string output1 = "Author1?author1@mail.com?47919e8f-7103-48a3-9514-3f2d9d49ac61\nAuthor2?author2@mail.com?"
						  "41ab7373-8f24-4a03-83dc-621036d99f34\n";
	std::string output2 = "Author2?author2@mail.com?41ab7373-8f24-4a03-83dc-621036d99f34\nAuthor1?author1@mail.com?"
						  "47919e8f-7103-48a3-9514-3f2d9d49ac61\n";
	Author author1;
	author1.name = "Author1";
	author1.mail = "author1@mail.com";
	Author author2;
	author2.name = "Author2";
	author2.mail = "author2@mail.com";

	EXPECT_CALL(database, idToAuthor("47919e8f-7103-48a3-9514-3f2d9d49ac61")).WillOnce(testing::Return(author1));
	EXPECT_CALL(database, idToAuthor("41ab7373-8f24-4a03-83dc-621036d99f34")).WillOnce(testing::Return(author2));
	std::string result = handler.handleRequest("idau", request, nullptr);
	ASSERT_TRUE(result == output1 || result == output2);
}

// Tests if program correctly retreieves an author with multiple requests and one match total.
TEST(GetAuthorRequest, MultipleRequestSingleMatch)
{
	MockDatabase database;
	RequestHandler handler;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, nullptr);

	std::string request = "47919e8f-7103-48a3-9514-3f2d9d49ac61\n41ab7373-8f24-4a03-83dc-621036d99f34\n";
	std::string output = "Author1?author1@mail.com?47919e8f-7103-48a3-9514-3f2d9d49ac61\n";
	Author author1;
	author1.name = "Author1";
	author1.mail = "author1@mail.com";
	Author author2;

	EXPECT_CALL(database, idToAuthor("47919e8f-7103-48a3-9514-3f2d9d49ac61")).WillOnce(testing::Return(author1));
	EXPECT_CALL(database, idToAuthor("41ab7373-8f24-4a03-83dc-621036d99f34")).WillOnce(testing::Return(author2));
	std::string result = handler.handleRequest("idau", request, nullptr);
	ASSERT_EQ(result, output);
}

// Tests if program correctly retreieves a method with one author and one match.
TEST(GetMethodByAuthorTests, SingleIdRequest)
{
	MockDatabase database;
	RequestHandler handler;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, nullptr);

	MethodId method;
	method.hash = "2c7f46d4f57cf9e66b03213358c7ddb5";
	method.projectId = 42;
	method.version = 69;

	std::vector<MethodId> v;
	v.push_back(method);

	std::string output = "41ab7373-8f24-4a03-83dc-621036d99f34?2c7f46d4f57cf9e66b03213358c7ddb5?42?69\n";

	EXPECT_CALL(database, authorToMethods("41ab7373-8f24-4a03-83dc-621036d99f34")).WillOnce(testing::Return(v));
	std::string result = handler.handleRequest("aume", "41ab7373-8f24-4a03-83dc-621036d99f34\n", nullptr);
	EXPECT_EQ(result, output);
}

// Tests if program correctly retreieves a method with multiple authors and one match per author.
TEST(GetMethodByAuthorTests, MultipleIdRequest)
{
	MockDatabase database;
	RequestHandler handler;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, nullptr);

	MethodId method1;
	method1.hash = "2c7f46d4f57cf9e66b03213358c7ddb5";
	method1.projectId = 42;
	method1.version = 69;

	MethodId method2;
	method2.hash = "06f73d7ab46184c55bf4742b9428a4c0";
	method2.projectId = 42;
	method2.version = 420;

	std::vector<MethodId> v1;
	v1.push_back(method1);
	std::vector<MethodId> v2;
	v2.push_back(method2);

	std::string output1 = "47919e8f-7103-48a3-9514-3f2d9d49ac61?2c7f46d4f57cf9e66b03213358c7ddb5?42?69\n41ab7373-8f24-"
						  "4a03-83dc-621036d99f34?06f73d7ab46184c55bf4742b9428a4c0?42?420\n";
	std::string output2 = "41ab7373-8f24-4a03-83dc-621036d99f34?06f73d7ab46184c55bf4742b9428a4c0?42?420\n47919e8f-7103-"
						  "48a3-9514-3f2d9d49ac61?2c7f46d4f57cf9e66b03213358c7ddb5?42?69\n";

	EXPECT_CALL(database, authorToMethods("47919e8f-7103-48a3-9514-3f2d9d49ac61")).WillOnce(testing::Return(v1));
	EXPECT_CALL(database, authorToMethods("41ab7373-8f24-4a03-83dc-621036d99f34")).WillOnce(testing::Return(v2));
	std::string result = handler.handleRequest("aume", "47919e8f-7103-48a3-9514-3f2d9d49ac61\n"
													   "41ab7373-8f24-4a03-83dc-621036d99f34\n", nullptr);
	EXPECT_TRUE(result == output1 || result == output2);
}

// Tests if program correctly retreieves a method with one author and no match.
TEST(GetMethodByAuthorTests, SingleIdNoMatch)
{
	MockDatabase database;
	RequestHandler handler;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, nullptr);

	std::vector<MethodId> v;

	EXPECT_CALL(database, authorToMethods("47919e8f-7103-48a3-9514-3f2d9d49ac61")).WillOnce(testing::Return(v));
	std::string result = handler.handleRequest("aume", "47919e8f-7103-48a3-9514-3f2d9d49ac61\n", nullptr);
	EXPECT_EQ(result, "No results found.");
}

// Tests if program correctly retreieves a method with multiple authors and one match total.
TEST(GetMethodByAuthorTests, MultipleIdOneMatch)
{
	MockDatabase database;
	RequestHandler handler;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, nullptr);

	MethodId method;
	method.hash = "2c7f46d4f57cf9e66b03213358c7ddb5";
	method.projectId = 42;
	method.version = 69;

	std::vector<MethodId> v;
	std::vector<MethodId> v2;
	v.push_back(method);

	EXPECT_CALL(database, authorToMethods("47919e8f-7103-48a3-9514-3f2d9d49ac61")).WillOnce(testing::Return(v2));
	EXPECT_CALL(database, authorToMethods("41ab7373-8f24-4a03-83dc-621036d99f34")).WillOnce(testing::Return(v));
	std::string result = handler.handleRequest("aume", "47919e8f-7103-48a3-9514-3f2d9d49ac61\n"
													   "41ab7373-8f24-4a03-83dc-621036d99f34\n", nullptr);

	std::string output = "41ab7373-8f24-4a03-83dc-621036d99f34?2c7f46d4f57cf9e66b03213358c7ddb5?42?69\n";

	EXPECT_EQ(result, output);
}

// Tests if program correctly retreieves a method with one author and multiple matches.
TEST(GetMethodByAuthorTests, OneIdMultipleMatches)
{
	MockDatabase database;
	RequestHandler handler;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, nullptr);

	MethodId method1;
	method1.hash = "2c7f46d4f57cf9e66b03213358c7ddb5";
	method1.projectId = 42;
	method1.version = 69;

	MethodId method2;
	method2.hash = "06f73d7ab46184c55bf4742b9428a4c0";
	method2.projectId = 42;
	method2.version = 420;

	std::vector<MethodId> v;
	v.push_back(method1);
	v.push_back(method2);

	std::string output1 = "47919e8f-7103-48a3-9514-3f2d9d49ac61?2c7f46d4f57cf9e66b03213358c7ddb5?42?69\n47919e8f-7103-"
						  "48a3-9514-3f2d9d49ac61?06f73d7ab46184c55bf4742b9428a4c0?42?420\n";
	std::string output2 = "47919e8f-7103-48a3-9514-3f2d9d49ac61?06f73d7ab46184c55bf4742b9428a4c0?42?420\n47919e8f-7103-"
						  "48a3-9514-3f2d9d49ac61?2c7f46d4f57cf9e66b03213358c7ddb5?42?69\n";

	EXPECT_CALL(database, authorToMethods("47919e8f-7103-48a3-9514-3f2d9d49ac61")).WillOnce(testing::Return(v));
	std::string result = handler.handleRequest("aume", "47919e8f-7103-48a3-9514-3f2d9d49ac61\n", nullptr);
	EXPECT_TRUE(result == output1 || result == output2);
}

// Tests if program correctly retreieves a method with multiple authors and multiple matches.
TEST(GetMethodByAuthorTests, MultipleIdsMultipleMatches)
{
	MockDatabase database;
	RequestHandler handler;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, nullptr);

	MethodId method1;
	method1.hash = "2c7f46d4f57cf9e66b03213358c7ddb5";
	method1.projectId = 42;
	method1.version = 69;

	MethodId method2;
	method2.hash = "06f73d7ab46184c55bf4742b9428a4c0";
	method2.projectId = 42;
	method2.version = 420;

	MethodId method3;
	method3.hash = "137fed017b6159acc0af30d2c6b403a5";
	method3.projectId = 69;
	method3.version = 420;

	std::vector<MethodId> v1;
	std::vector<MethodId> v2;
	v1.push_back(method1);
	v1.push_back(method2);
	v2.push_back(method3);

	EXPECT_CALL(database, authorToMethods("47919e8f-7103-48a3-9514-3f2d9d49ac61")).WillOnce(testing::Return(v1));
	EXPECT_CALL(database, authorToMethods("41ab7373-8f24-4a03-83dc-621036d99f34")).WillOnce(testing::Return(v2));
	std::string result = handler.handleRequest("aume", "47919e8f-7103-48a3-9514-3f2d9d49ac61\n"
													   "41ab7373-8f24-4a03-83dc-621036d99f34\n", nullptr);

	std::string output1 = "47919e8f-7103-48a3-9514-3f2d9d49ac61?2c7f46d4f57cf9e66b03213358c7ddb5?42?69\n";
	std::string output2 = "47919e8f-7103-48a3-9514-3f2d9d49ac61?06f73d7ab46184c55bf4742b9428a4c0?42?420\n";
	std::string output3 = "41ab7373-8f24-4a03-83dc-621036d99f34?137fed017b6159acc0af30d2c6b403a5?69?420\n";

	EXPECT_EQ(result.size(), output1.size() + output2.size() + output3.size());
	EXPECT_TRUE(result.find(output1) != std::string::npos);
	EXPECT_TRUE(result.find(output2) != std::string::npos);
	EXPECT_TRUE(result.find(output3) != std::string::npos);
}
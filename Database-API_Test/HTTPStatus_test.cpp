/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/
#include "HTTPStatus.h"

#include <gtest/gtest.h>

// Check if successful status codes works as intended for a normal message.
TEST(HTTPsuccesMessage, regularString)
{
	std::string message = "This is a normal message.";
	std::string expected = std::string("200") + ENTRY_DELIMITER_CHAR + message;

	EXPECT_EQ(HTTPStatusCodes::success(message), expected);
}

// Check if successful status codes works as intended for an empty message.
TEST(HTTPsuccesMessage, emptyString)
{
	std::string message = "";
	std::string expected = std::string("200") + ENTRY_DELIMITER_CHAR + message;

	EXPECT_EQ(HTTPStatusCodes::success(message), expected);
}

// Check if successful status codes works as intended for a message with odd characters.
TEST(HTTPsuccesMessage, oddCharacters)
{
	std::string message = "これは例文です。正確に表示されたら嬉しいです。";
	std::string expected = std::string("200") + ENTRY_DELIMITER_CHAR + message;

	EXPECT_EQ(HTTPStatusCodes::success(message), expected);
}

// Check if client error status codes works as intended for a normal message.
TEST(HTTPclientMessage, regularString)
{
	std::string message = "This is a normal message.";
	std::string expected = std::string("400") + ENTRY_DELIMITER_CHAR + message;

	EXPECT_EQ(HTTPStatusCodes::clientError(message), expected);
}

// Check if client error status codes works as intended for an empty message.
TEST(HTTPclientMessage, emptyString)
{
	std::string message = "";
	std::string expected = std::string("400") + ENTRY_DELIMITER_CHAR + message;

	EXPECT_EQ(HTTPStatusCodes::clientError(message), expected);
}

// Check if client error status codes works as intended for a message with odd characters.
TEST(HTTPclientMessage, oddCharacters)
{
	std::string message = "これは例文です。正確に表示されたら嬉しいです。";
	std::string expected = std::string("400") + ENTRY_DELIMITER_CHAR + message;

	EXPECT_EQ(HTTPStatusCodes::clientError(message), expected);
}

// Check if server error status codes works as intended for a normal message.
TEST(HTTPServerMessage, regularString)
{
	std::string message = "This is a normal message.";
	std::string expected = std::string("500") + ENTRY_DELIMITER_CHAR + message;

	EXPECT_EQ(HTTPStatusCodes::serverError(message), expected);
}

// Check if server error status codes works as intended for an empty message.
TEST(HTTPServerMessage, emptyString)
{
	std::string message = "";
	std::string expected = std::string("500") + ENTRY_DELIMITER_CHAR + message;

	EXPECT_EQ(HTTPStatusCodes::serverError(message), expected);
}

// Check if server error status codes works as intended for an odd message.
TEST(HTTPServerMessage, oddCharacters)
{
	std::string message = "これは例文です。正確に表示されたら嬉しいです。";
	std::string expected = std::string("500") + ENTRY_DELIMITER_CHAR + message;

	EXPECT_EQ(HTTPStatusCodes::serverError(message), expected);
}

// Check if we are able to obtain the code of a normal message.
TEST(HTTPgetCode, regularRequest)
{
	std::string code = "200";
	std::string request = code + ENTRY_DELIMITER_CHAR + "This is a normal message.";

	EXPECT_EQ(HTTPStatusCodes::getCode(request), code);
}

// Check if we are able to obtain the code of a normal request.
TEST(HTTPgetCode, noCode)
{
	std::string code = "";
	std::string request = code + ENTRY_DELIMITER_CHAR + "This is a normal message.";

	EXPECT_EQ(HTTPStatusCodes::getCode(request), "500"); 
}

// Check if we are able to obtain the message of a normal request.
TEST(HTTPgetMessage, regularRequest)
{
	std::string message = "This is a normal message.";
	std::string request = std::string("200") + ENTRY_DELIMITER_CHAR + message;

	EXPECT_EQ(HTTPStatusCodes::getMessage(request), message);
}

// Check if we are able to obtain the message of an empty request.
TEST(HTTPgetMessage, noCode)
{
	std::string message = "";
	std::string request = std::string("200") + ENTRY_DELIMITER_CHAR + message;

	EXPECT_EQ(HTTPStatusCodes::getMessage(request), message);
}
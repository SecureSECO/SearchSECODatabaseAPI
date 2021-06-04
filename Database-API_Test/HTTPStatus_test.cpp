/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/
#include "HTTPStatus.h"

#include <gtest/gtest.h>

// Success.
TEST(HTTPsuccesMessage, regularString)
{
    std::string message = "This is a normal message.";
    std::string expected = std::string("200") + ENTRY_DELIMITER_CHAR + message;

    EXPECT_EQ(HTTPStatusCodes::success(message), expected);
}

TEST(HTTPsuccesMessage, emptyString)
{
    std::string message = "";
	std::string expected = std::string("200") + ENTRY_DELIMITER_CHAR + message;

    EXPECT_EQ(HTTPStatusCodes::success(message), expected);
}

TEST(HTTPsuccesMessage, oddCharacters)
{
    std::string message = "これは例文です。正確に表示されたら嬉しいです。";
	std::string expected = std::string("200") + ENTRY_DELIMITER_CHAR + message;

    EXPECT_EQ(HTTPStatusCodes::success(message), expected);
}

// ClientError.
TEST(HTTPclientMessage, regularString)
{
    std::string message = "This is a normal message.";
	std::string expected = std::string("400") + ENTRY_DELIMITER_CHAR + message;

    EXPECT_EQ(HTTPStatusCodes::clientError(message), expected);
}

TEST(HTTPclientMessage, emptyString)
{
    std::string message = "";
	std::string expected = std::string("400") + ENTRY_DELIMITER_CHAR + message;

    EXPECT_EQ(HTTPStatusCodes::clientError(message), expected);
}

TEST(HTTPclientMessage, oddCharacters)
{
    std::string message = "これは例文です。正確に表示されたら嬉しいです。";
	std::string expected = std::string("400") + ENTRY_DELIMITER_CHAR + message;

    EXPECT_EQ(HTTPStatusCodes::clientError(message), expected);
}

// ServerError.
TEST(HTTPServerMessage, regularString)
{
    std::string message = "This is a normal message.";
	std::string expected = std::string("500") + ENTRY_DELIMITER_CHAR + message;

    EXPECT_EQ(HTTPStatusCodes::serverError(message), expected);
}

TEST(HTTPServerMessage, emptyString)
{
    std::string message = "";
	std::string expected = std::string("500") + ENTRY_DELIMITER_CHAR + message;

    EXPECT_EQ(HTTPStatusCodes::serverError(message), expected);
}

TEST(HTTPServerMessage, oddCharacters)
{
    std::string message = "これは例文です。正確に表示されたら嬉しいです。";
	std::string expected = std::string("500") + ENTRY_DELIMITER_CHAR + message;

    EXPECT_EQ(HTTPStatusCodes::serverError(message), expected);
}

// GetCode.
TEST(HTTPgetCode, regularRequest)
{
    std::string code = "200";
	std::string request = code + ENTRY_DELIMITER_CHAR + "This is a normal message.";

    EXPECT_EQ(HTTPStatusCodes::getCode(request), code);
}

TEST(HTTPgetCode, noCode)
{
    std::string code = "";
	std::string request = code + ENTRY_DELIMITER_CHAR + "This is a normal message.";

    EXPECT_EQ(HTTPStatusCodes::getCode(request), "500"); 
}

// GetMessage
TEST(HTTPgetMessage, regularRequest)
{
    std::string message = "This is a normal message.";
	std::string request = std::string("200") + ENTRY_DELIMITER_CHAR + message;

    EXPECT_EQ(HTTPStatusCodes::getMessage(request), message);
}

TEST(HTTPgetMessage, noCode)
{
    std::string message = "";
	std::string request = std::string("200") + ENTRY_DELIMITER_CHAR + message;

    EXPECT_EQ(HTTPStatusCodes::getMessage(request), message);
}
/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/


#include "HTTPStatus.h"
#include "Utility.h"

#define DELIMITER '\n'

enum HTTPStatusCode
{
	successCode = 200,
	clientErrorCode = 400,
};


std::string constructMessage(HTTPStatusCode code, std::string message)
{
	return std::to_string(code) + DELIMITER + message;
}

std::string HTTPStatusCodes::success(std::string message)
{
	return constructMessage(successCode, message);
}

std::string HTTPStatusCodes::clientError(std::string message)
{
	return constructMessage(clientErrorCode, message);
}

std::string HTTPStatusCodes::getCode(std::string request)
{
	return Utility::splitStringOn(request, '\n')[0];
}

std::string HTTPStatusCodes::getMessage(std::string request)
{
	return request.substr(4, request.length() - 4);
}

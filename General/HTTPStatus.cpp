/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/


#include "HTTPStatus.h"
#include "Utility.h"

enum HTTPStatusCode
{
	successCode = 200,
	clientErrorCode = 400,
	serverErrorCode = 500
};


std::string constructMessage(HTTPStatusCode code, std::string message)
{
	return std::to_string(code) + ENTRY_DELIMITER_CHAR + message;
}

bool validCode(std::string request)
{
	std::string code = Utility::splitStringOn(request, ENTRY_DELIMITER_CHAR)[0];

	return code == std::to_string(successCode) 
		|| code == std::to_string(clientErrorCode) 
		|| code == std::to_string(serverErrorCode);
}

std::string HTTPStatusCodes::success(std::string message)
{
	return constructMessage(successCode, message);
}

std::string HTTPStatusCodes::clientError(std::string message)
{
	return constructMessage(clientErrorCode, message);
}

std::string HTTPStatusCodes::serverError(std::string message)
{
	return constructMessage(serverErrorCode, message);
}

std::string HTTPStatusCodes::getCode(std::string request)
{
	if (validCode(request))
	{
		return Utility::splitStringOn(request, ENTRY_DELIMITER_CHAR)[0];
	}
	else
	{
		return std::to_string(serverErrorCode);
	}
}

std::string HTTPStatusCodes::getMessage(std::string request)
{
	if (validCode(request))
	{
		return request.substr(4, request.length() - 4);
	}
	else
	{
		return request;
	}
}

/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "RequestHandler.h"

using namespace std;

void RequestHandler::initialize(DatabaseHandler *databaseHandler, std::string ip, int port)
{
	// Make the requestHandlers.
	dbrh = new DatabaseRequestHandler(databaseHandler, ip, port);
	jrh  = JobRequestHandler();
}

string RequestHandler::handleRequest(string requestType, string request)
{
	// We convert the requestType to a eRequestType (for the switch below).
	eRequestType eRequestType = getERequestType(requestType);

	// We handle the request based on its type.
	string result;
	switch (eRequestType)
	{
		case eUpload:
			result = dbrh->handleUploadRequest(request);
			break;
		case eCheck:
			result = dbrh->handleCheckRequest(request);
			break;
		case eCheckUpload:
			result = dbrh->handleCheckUploadRequest(request);
			break;
		case eGetAuthorID:
			result = dbrh->handleGetAuthorIDRequest(request);
			break;
		case eGetAuthor:
			result = dbrh->handleGetAuthorRequest(request);
			break;
		case eGetMethodByAuthor:
			result = dbrh->handleGetMethodsByAuthorRequest(request);
			break;
		case eUnknown:
			result = handleUnknownRequest();
			break;
		default:
			result = handleNotImplementedRequest();
			break;
	}
	return result;
}

string RequestHandler::handleUnknownRequest()
{
	return "Unknown request type.";
}

string RequestHandler::handleNotImplementedRequest()
{
	return "Request not implemented yet.";
}

eRequestType RequestHandler::getERequestType(string requestType)
{
	if (requestType == "upld")
	{
		return eUpload;
	}
	else if (requestType == "chck")
	{
		return eCheck;
	}
	else if (requestType == "chup")
	{
		return eCheckUpload;
	}
	else if (requestType == "auid")
	{
		return eGetAuthorID;
	}
	else if (requestType == "idau")
	{
		return eGetAuthor;
	}
	else if (requestType == "aume")
	{
		return eGetMethodByAuthor;
	}
	else
	{
		return eUnknown;
	}
}

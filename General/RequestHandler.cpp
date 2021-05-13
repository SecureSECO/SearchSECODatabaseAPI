#include "RequestHandler.h"

using namespace std;

void RequestHandler::initialize(DatabaseHandler *databaseHandler, DatabaseConnection *databaseConnection, std::string ip, int port)
{
	// Make the requestHandlers.
	dbrh = new DatabaseRequestHandler(databaseHandler, ip, port);
	jrh  = new JobRequestHandler(databaseConnection, ip, port);
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
		case eUploadJob:
			result = jrh->handleUploadJobRequest(request);
			break;
		case eGetTopJob:
			result = jrh->handleGetJobRequest();
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
	else if (requestType == "upjb")
	{
		return eUploadJob;
	}
	else if (requestType == "tpjb")
	{
		return eGetTopJob;
	}
	else
	{
		return eUnknown;
	}
}

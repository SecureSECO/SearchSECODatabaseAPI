#include "RequestHandler.h"
#include "DatabaseRequestHandler.h"

using namespace std;

void RequestHandler::initialize(DatabaseHandler *databaseHandler, std::string ip, int port)
{
	// Set up a connection with the database.
	database = databaseHandler;
	database -> connect(ip, port);
}

string RequestHandler::handleRequest(string requestType, string request)
{
	// We convert the requestType to a eRequestType (for the switch below).
	eRequestType eRequestType = getERequestType(requestType);

    // Make the requestHandlers
    DatabaseRequestHandler dbrh = DatabaseRequestHandler(databaseHandler);

	// We handle the request based on its type.
	string result;
	switch (eRequestType)
	{
		case eUpload:
			result = dbrh.handleUploadRequest(request);
			break;
		case eCheck:
			result = dbrh.handleCheckRequest(request);
			break;
		case eCheckUpload:
			result = dbrh.handleCheckUploadRequest(request);
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

string DatabaseRequestHandler::handleUnknownRequest()
{
	return "Unknown request type.";
}

string DatabaseRequestHandler::handleNotImplementedRequest()
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
	else
	{
		return eUnknown;
	}
}

#include "RequestHandler.h"

using namespace std;

void RequestHandler::initialize(DatabaseHandler *databaseHandler, DatabaseConnection *databaseConnection, RAFTConsensus* raft, std::string ip, int port)
{
	// Make the requestHandlers.
	dbrh = new DatabaseRequestHandler(databaseHandler, ip, port);
	jrh  = new JobRequestHandler(databaseConnection, raft, this, ip, port);
}

string RequestHandler::handleRequest(string requestType, string request, boost::shared_ptr<TcpConnection> connection)
{
	// We convert the requestType to a eRequestType (for the switch below).
	eRequestType eRequest = getERequestType(requestType);

	// We handle the request based on its type.
	string result;
	switch (eRequest)
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
		case eConnect:
			result = jrh->handleConnectRequest(connection, request);
			break;
		case eUploadJob:
			result = jrh->handleUploadJobRequest(requestType, request);
			break;
		case eGetTopJob:
			result = jrh->handleGetJobRequest(requestType, request);
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
	else if (requestType == "conn")
	{
		return eConnect;
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

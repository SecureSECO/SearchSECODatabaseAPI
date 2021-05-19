/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "RequestHandler.h"
#include <iostream>

using namespace std;

void RequestHandler::initialize(DatabaseHandler *databaseHandler, DatabaseConnection *databaseConnection, RAFTConsensus* raft, std::string ip, int port)
{
	// Make the requestHandlers.
	cout << "Begin initialize\n";
	dbrh = new DatabaseRequestHandler(databaseHandler, ip, port);
	cout << "Database initialized\n";
	jrh  = new JobRequestHandler(raft, this, databaseConnection, ip, port);
	cout << "Job initialize done\n";
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
		case eUploadCrawlData:
			result = jrh->handleCrawlDataRequest(requestType, request);
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
	else if (requestType == "upcd")
	{
		return eUploadCrawlData;
	}
	else if (requestType == "gtjb")
	{
		return eGetTopJob;
	}
	else
	{
		return eUnknown;
	}
}

/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "RequestHandler.h"
#include <iostream>

void RequestHandler::initialize(DatabaseHandler *databaseHandler, DatabaseConnection *databaseConnection, RAFTConsensus* raft, std::string ip, int port)
{
	// Make the requestHandlers.
	dbrh = new DatabaseRequestHandler(databaseHandler, ip, port);
	jrh  = new JobRequestHandler(raft, this, databaseConnection, ip, port);
}

std::string RequestHandler::handleRequest(std::string requestType, std::string request, boost::shared_ptr<TcpConnection> connection)
{
	// We convert the requestType to a eRequestType (for the switch below).
	eRequestType eRequest = getERequestType(requestType);

	// We handle the request based on its type.
	std::string result;
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
		case eExtractProjects:
			result = dbrh->handleExtractProjectsRequest(request);
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

std::string RequestHandler::handleUnknownRequest()
{
	return "Unknown request type.";
}

std::string RequestHandler::handleNotImplementedRequest()
{
	return "Request not implemented yet.";
}

eRequestType RequestHandler::getERequestType(std::string requestType)
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
	else if (requestType == "extp")
	{
		return eExtractProjects;
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

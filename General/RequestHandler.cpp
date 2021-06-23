/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "HTTPStatus.h"
#include "RequestHandler.h"

void RequestHandler::initialize(DatabaseHandler *databaseHandler, DatabaseConnection *databaseConnection, RAFTConsensus* raft, std::string ip, int port)
{
	// Initialise the requestHandlers.
	dbrh = new DatabaseRequestHandler(databaseHandler, ip, port);
	jrh  = new JobRequestHandler(raft, this, databaseConnection, ip, port);
}

std::string RequestHandler::handleRequest(std::string requestType, std::string request, boost::shared_ptr<TcpConnection> connection)
{
	ERequestType eRequest = getERequestType(requestType);

	// Handle the request based on its type.
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
		case eGetAuthor:
			result = dbrh->handleGetAuthorRequest(request);
			break;
		case eGetMethodByAuthor:
			result = dbrh->handleGetMethodsByAuthorRequest(request);
			break;
		case eGetPrevProjectsRequest:
			result = dbrh->handlePrevProjectsRequest(request);
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
	return HTTPStatusCodes::clientError("Unknown request type.");
}

std::string RequestHandler::handleNotImplementedRequest()
{
	return HTTPStatusCodes::clientError("Request not implemented yet.");
}

ERequestType RequestHandler::getERequestType(std::string requestType)
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
	else if (requestType == "idau")
	{
		return eGetAuthor;
	}
	else if (requestType == "aume")
	{
		return eGetMethodByAuthor;
	}
	else if (requestType == "gppr")
	{
		return eGetPrevProjectsRequest;
	}
	else
	{
		return eUnknown;
	}
}

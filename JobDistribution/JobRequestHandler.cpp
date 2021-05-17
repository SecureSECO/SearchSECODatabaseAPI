/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "JobRequestHandler.h"


#include <iostream>

JobRequestHandler::JobRequestHandler(RAFTConsensus* raft, RequestHandler* requestHandler, DatabaseConnection* database, std::string ip, int port)
{
	this->raft = raft;
	this->requestHandler = requestHandler;
	this->database = database;
	database->connect(ip, port);
}

std::string JobRequestHandler::handleConnectRequest(boost::shared_ptr<TcpConnection> connection, std::string request)
{
	return raft->connectNewNode(connection, request);
}

std::string JobRequestHandler::handleGetJobRequest(std::string request, std::string data)
{
	if (raft->isLeader())
	{
		return database->getJob();
	}
	return raft->passRequestToLeader(request, data);
}

std::string JobRequestHandler::handleUploadJobRequest(std::string request, std::string data) //Data format is url1?priority1?url2?priority2?...
{
	if (raft->isLeader())
	{
		std::vector<std::string> datasplits = Utility::splitStringOn(data,'?');
		for (int i = 0; i < datasplits.size() / 2; i++)
		{
			std::string url = datasplits[2 * i];
			int priority = Utility::safeStoi(datasplits[2 * i + 1]);
			database->uploadJob(url, priority);
		}
		if (errno == 0)
		{
			return "Your job(s) has been succesfully added to the queue.";
		}
		else
		{
			return "An error has occurred while adding your job(s) to the queue.";
		}
	}
	return raft->passRequestToLeader(request, data);
}

std::string JobRequestHandler::handleCrawlDataRequest(std::string request, std::string data) //Data format is id?url1?priority1?url2?priority2?...
{
	if (raft->isLeader())
        {
		int crawlId = Utility::safeStoi(data.substr(0, data.find('?')));
		database->updateCrawlId(crawlId);
		std::string jobdata = data.substr(data.find('?'), data.length() - 1);
		return handleUploadJobRequest(request, jobdata);
	}
	return raft->passRequestToLeader(request, data);
}

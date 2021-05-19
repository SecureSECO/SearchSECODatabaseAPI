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

std::string JobRequestHandler::handleUploadJobRequest(std::string request, std::string data) // Data format is url1?priority1?url2?priority2?...
{
	if (raft->isLeader())
	{
		// Split data on '?'.
		std::vector<std::string> datasplits = Utility::splitStringOn(data,'?');
		std::vector<std::string> urls;
		std::vector<int> priorities;
		for (int i = 0; i < datasplits.size() / 2; i++)
		{
			std::string url = datasplits[2 * i];
			urls.push_back(url);
			int priority = Utility::safeStoi(datasplits[2 * i + 1]);
			// Check if priority could be parsed correctly.
			if (errno == 0)
			{
				priorities.push_back(priority);
			}
			else
			{
				return "A job has an invalid priority, no jobs have been added to the queue.";
			}
		}
		// Call to the database to upload jobs.
		for (int i = 0; i < urls.size(); i++)
		{
			database->uploadJob(urls[i], priorities[i]);
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

std::string JobRequestHandler::handleCrawlDataRequest(std::string request, std::string data) // Data format is id?url1?priority1?url2?priority2?...
{
	if (raft->isLeader())
        {
		int crawlId = Utility::safeStoi(data.substr(0, data.find('?')));
		if (errno == 0)
		{
			database->updateCrawlId(crawlId);
		}
		else
		{
			return "Error: invalid crawlId.";
		}
		// Get data after crawlId and pass it on to handleUploadRequest.
		std::string jobdata = data.substr(data.find('?') + 1, data.length());
		return handleUploadJobRequest(request, jobdata);
	}
	return raft->passRequestToLeader(request, data);
}

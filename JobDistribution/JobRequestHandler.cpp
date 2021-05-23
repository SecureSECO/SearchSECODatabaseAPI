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
	numberOfJobs = database->getNumberOfJobs();
	crawlId = 0;
}

std::string JobRequestHandler::handleConnectRequest(boost::shared_ptr<TcpConnection> connection, std::string request)
{
	return raft->connectNewNode(connection, request);
}

std::string JobRequestHandler::handleGetJobRequest(std::string request, std::string data)
{
	if (raft->isLeader())
	{
		// Check if number of jobs is enough to provide the top job.
        	if (numberOfJobs >= MIN_AMOUNT_JOBS || (alreadyCrawling == true && numberOfJobs >= 1))
        	{
			numberOfJobs -= 1;
                	return "Spider?" +  database->getTopJob();
        	}
        	// If number of jobs is not high enough, the job is to crawl for more jobs.
        	else if (alreadyCrawling == false)
        	{
                	alreadyCrawling = true;
                	return "Crawl?" + std::to_string(crawlId);
        	}
        	else
        	{
                	return "NoJob";
        	}
	}
	return raft->passRequestToLeader(request, data);
}

std::string JobRequestHandler::handleUploadJobRequest(std::string request, std::string data) // Data format is url1?priority1\nurl2?priority2\n...
{
	if (raft->isLeader())
	{
		// Split data on '\n', to get individual jobs.
		std::vector<std::string> datasplits = Utility::splitStringOn(data,'\n');
		std::vector<std::string> urls;
		std::vector<int> priorities;
		for (int i = 0; i < datasplits.size(); i++)
		{
			std::vector<std::string> dataSecondSplit = Utility::splitStringOn(datasplits[i],'?');
			std::string url = dataSecondSplit[0];
			urls.push_back(url);
			int priority = Utility::safeStoi(dataSecondSplit[1]);

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
			numberOfJobs += 1;
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

std::string JobRequestHandler::handleCrawlDataRequest(std::string request, std::string data) // Data format is id\nurl1?priority1\nurl2?priority2\n...
{
	if (raft->isLeader())
        {
		int id = Utility::safeStoi(data.substr(0, data.find('\n')));
		std::cout << std::to_string(id) + "\n";
		if (errno == 0)
		{
			updateCrawlId(id);
		}
		else
		{
			return "Error: invalid crawlId.";
		}
		// Get data after crawlId and pass it on to handleUploadRequest.
		std::string jobdata = data.substr(data.find('\n') + 1, data.length());
		return handleUploadJobRequest(request, jobdata);
	}
	return raft->passRequestToLeader(request, data);
}

void JobRequestHandler::updateCrawlId(int id)
{
        crawlId = id;
        alreadyCrawling = false;
}

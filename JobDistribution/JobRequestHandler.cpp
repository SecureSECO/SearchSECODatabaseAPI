/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "JobRequestHandler.h"
#include "HTTPStatus.h"

#include <iostream>
#include <exception>

JobRequestHandler::JobRequestHandler(RAFTConsensus* raft, RequestHandler* requestHandler, DatabaseConnection* database, std::string ip, int port)
{
	this->raft = raft;
	this->requestHandler = requestHandler;
	this->database = database;
	connectWithRetry(ip, port);
	numberOfJobs = database->getNumberOfJobs();
	std::cout << "Print debug1\n";
	timeLastRecount = Utility::getCurrentTimeSeconds();
	std::cout << "Print debug2\n";
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
		auto currentTime = Utility::getCurrentTimeSeconds();
		bool alreadyCrawling = true;
		if (timeLastCrawl == -1 || currentTime - timeLastCrawl > CRAWL_TIMEOUT_SECONDS) 
		{
			alreadyCrawling = false;
		}
		// Check if number of jobs is enough to provide the top job.
		if (numberOfJobs >= MIN_AMOUNT_JOBS || (alreadyCrawling == true && numberOfJobs >= 1))
		{
			return getTopJobWithRetry();
		}
		// If number of jobs is not high enough, the job is to crawl for more jobs.
		else if (alreadyCrawling == false)
		{
			timeLastCrawl = currentTime;
			std::string s = "Crawl";
			s += FIELD_DELIMITER_CHAR;
			return HTTPStatusCodes::success(s + std::to_string(crawlId));
		}
		else
		{
			return HTTPStatusCodes::success("NoJob");
		}
	}
	return raft->passRequestToLeader(request, data);
}

std::string JobRequestHandler::handleUploadJobRequest(std::string request, std::string data)
{
	if (raft->isLeader())
	{
		// Split data on ENTRY_DELIMITER_CHAR, to get individual jobs.
		std::vector<std::string> datasplits = Utility::splitStringOn(data, ENTRY_DELIMITER_CHAR);
		std::vector<std::string> urls;
		std::vector<int> priorities;
		for (int i = 0; i < datasplits.size(); i++)
		{
			std::vector<std::string> dataSecondSplit = Utility::splitStringOn(datasplits[i], FIELD_DELIMITER_CHAR);
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
				return HTTPStatusCodes::clientError("A job has an invalid priority, no jobs have been added to the queue.");
			}
		}
		// Call to the database to upload jobs.
		for (int i = 0; i < urls.size(); i++)
		{
			if (tryUploadJobWithRetry(urls[i], priorities[i]))
			{
				numberOfJobs += 1;
			}
			else
			{
				return HTTPStatusCodes::serverError("Unable to add job " + std::to_string(i) + " to database.");
			}
		}

		long long timeNow = Utility::getCurrentTimeSeconds();
		if (timeNow - timeLastRecount > RECOUNT_WAIT_TIME) 
		{
			numberOfJobs = database->getNumberOfJobs();
		}

		if (errno == 0)
		{
			return HTTPStatusCodes::success("Your job(s) has been succesfully added to the queue.");
		}
		else
		{
			return HTTPStatusCodes::clientError("An error has occurred while adding your job(s) to the queue.");
		}
	}
	return raft->passRequestToLeader(request, data);
}

std::string JobRequestHandler::handleCrawlDataRequest(std::string request, std::string data)
{
	if (raft->isLeader())
	{
		int id = Utility::safeStoi(data.substr(0, data.find(ENTRY_DELIMITER_CHAR)));
		if (errno == 0)
		{
			updateCrawlId(id);
		}
		else
		{
			return HTTPStatusCodes::clientError("Error: invalid crawlId.");
		}
		// Get data after crawlId and pass it on to handleUploadRequest.
		std::string jobdata = data.substr(data.find(ENTRY_DELIMITER_CHAR) + 1, data.length());
		return handleUploadJobRequest(request, jobdata);
	}
	return raft->passRequestToLeader(request, data);
}

void JobRequestHandler::updateCrawlId(int id)
{
	crawlId = id;
	timeLastCrawl = -1;
}

void JobRequestHandler::connectWithRetry(std::string ip, int port)
{
	errno = 0;
	database->connect(ip, port);
	int retries = 0;
	if (errno != 0)
	{
		while (retries < MAX_RETRIES)
		{
			database->connect(ip, port);
			if (errno == 0)
			{
				return;
			}
			retries++;
		}
		throw "Unable to connect to database.";
	}
	errno = 0;
}

std::string JobRequestHandler::getTopJobWithRetry()
{
	std::string url = database->getTopJob();
	int retries = 0;
	if (errno != 0)
	{
		while (retries < MAX_RETRIES)
		{
			url = database->getTopJob();
			if (errno == 0)
			{
				return HTTPStatusCodes::success(std::string("Spider") + FIELD_DELIMITER_CHAR + url);
			}
			retries++;
		}
		return HTTPStatusCodes::serverError("Unable to get job from database.");
	}
	numberOfJobs -= 1;
	return HTTPStatusCodes::success(std::string("Spider") + FIELD_DELIMITER_CHAR + url);
}

bool JobRequestHandler::tryUploadJobWithRetry(std::string url, int priority)
{
	int retries = 0;
	database->uploadJob(url, priority);
	if (errno != 0)
	{
		while (retries < MAX_RETRIES)
		{
			database->uploadJob(url, priority);
			if (errno == 0)
			{
				return true;
			}
			retries++;
		}
		return false;
	}
	return true;
}

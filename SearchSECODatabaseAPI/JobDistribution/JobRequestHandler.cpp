/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "JobRequestHandler.h"
#include "Definitions.h"
#include "HTTPStatus.h"
#include "Utility.h"

JobRequestHandler::JobRequestHandler(RAFTConsensus *raft, RequestHandler *requestHandler, DatabaseConnection *database,
									 Statistics *stats, std::string ip, int port)
{
	this->raft = raft;
	this->requestHandler = requestHandler;
	this->database = database;
	this->stats = stats;
	connectWithRetry(ip, port);
	numberOfJobs = database->getNumberOfJobs();
	crawlID = database->getCrawlID();
	timeLastRecount = Utility::getCurrentTimeSeconds();
}

std::string JobRequestHandler::handleConnectRequest(boost::shared_ptr<TcpConnection> connection, std::string request)
{
	return raft->connectNewNode(connection, request);
}

std::string JobRequestHandler::handleGetIPsRequest(std::string request, std::string client, std::string data)
{
	// If you are the leader, get the ips from the RAFT consensus.
	if (raft->isLeader())
	{
		std::vector<std::string> ips = raft->getCurrentIPs();
		std::string result;
		for (std::string ip : ips)
		{
			result += ip + ENTRY_DELIMITER_CHAR;
		}
		return HTTPStatusCodes::success(result);
	}
	// If you are not the leader, pass the request to the leader.
	return raft->passRequestToLeader(request, client, data);
}

std::string JobRequestHandler::handleGetJobRequest(std::string request, std::string client, std::string data)
{
	// If you are the leader, check if there is a job left.
	if (raft->isLeader())
	{
		// Lock job mutex, to prevent two threads simultaneously handing out the same job.
		jobmtx.lock();
		auto currentTime = Utility::getCurrentTimeSeconds();
		bool alreadyCrawling = true;
		if (timeLastCrawl == -1 || currentTime - timeLastCrawl > CRAWL_TIMEOUT_SECONDS)
		{
			alreadyCrawling = false;
		}
		// Check if number of jobs is enough to provide the top job.
		if (numberOfJobs >= MIN_AMOUNT_JOBS || (alreadyCrawling == true && numberOfJobs >= 1))
		{
			std::string job = getTopJobWithRetry();
			jobmtx.unlock();
			return job;
		}
		// If the number of jobs is not high enough, the job is to crawl for more jobs.
		else if (alreadyCrawling == false)
		{
			timeLastCrawl = currentTime;
			std::string s = "Crawl";
			s += FIELD_DELIMITER_CHAR;
			s += std::to_string(crawlID);

			// Identifier for this crawl job.
			s += FIELD_DELIMITER_CHAR;
			s += std::to_string(timeLastCrawl);
			jobmtx.unlock();
			return HTTPStatusCodes::success(s);
		}
		else
		{
			jobmtx.unlock();
			return HTTPStatusCodes::success("NoJob");
		}
	}
	// If you are not the leader, pass the request to the leader.
	return raft->passRequestToLeader(request, client, data);
}

std::string JobRequestHandler::handleFinishJobRequest(std::string request, std::string client, std::string data)
{
	// If you are the leader, check if there is a job left.
	if (raft->isLeader())
	{
		std::vector<std::string> splitted = Utility::splitStringOn(data, FIELD_DELIMITER_CHAR);
		std::string jobid = splitted[0];
		long long jobTime = Utility::safeStoll(splitted[1]);
		int reasonID = Utility::safeStoi(splitted[2]);
		std::string reasonData = splitted[3];

		auto job = getCurrentJobWithRetry(jobid, jobTime);//TODO

		// If job was not present in currentjobs,
		if (job == nullptr)
		{
			// a newer job was already given out. Signal this to the worker.
			std::cout << "Received unknown job with ID: " << jobid << std::endl;

			return HTTPStatusCodes::clientError("Job not currently expected.");
		}

		// Check if the worker failed to complete the job.
		if (reasonID != 0)
		{
			addFailedJobWithRetry(jobid, jobTime, job.priority, job.url, job.retries, reasonID, reasonData);

			if (errno != 0)
			{
				std::cout << "Could not store failed job: " << data << std::endl;
				return HTTPStatusCodes::databaseError("Job could not be added to failed jobs list.");
			}

			return HTTPStatusCodes::success("Job failed succesfully.");
		}
		else
		{
			return HTTPStatusCodes::success("Job finished succesfully.");
		}
	}
	// If you are not the leader, pass the request to the leader.
	return raft->passRequestToLeader(request, client, data);
}

std::string JobRequestHandler::handleUploadJobRequest(std::string request, std::string client, std::string data)
{
	if (raft->isLeader())
	{
		return handleUploadJobRequest(request, client, Utility::splitStringOn(data, ENTRY_DELIMITER_CHAR));
	}
	return raft->passRequestToLeader(request, client, data);
}

std::string JobRequestHandler::handleUploadJobRequest(std::string request, std::string client,
													  std::vector<std::string> data)
{
	std::vector<std::string> urls;
	std::vector<int> priorities;
	std::vector<long long> timeouts;
	for (int i = 0; i < data.size(); i++)
	{
		std::vector<std::string> dataSecondSplit = Utility::splitStringOn(data[i], FIELD_DELIMITER_CHAR);
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

		long long timeout = Utility::safeStoll(dataSecondSplit[2]);

		// Check if timeout could be parsed correctly.
		if (errno == 0)
		{
			timeouts.push_back(timeout);
		}
		else
		{
			return HTTPStatusCodes::clientError("A job has an invalid timeout, no jobs have been added to the queue.");
		}
	}

	// Call to the database to upload jobs.
	for (int i = 0; i < urls.size(); i++)
	{
		if (tryUploadJobWithRetry(urls[i], priorities[i], 0, timeouts[i]))
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

std::string JobRequestHandler::handleCrawlDataRequest(std::string request, std::string client, std::string data)
{
	// If this node is the leader, we handle the request, otherwise, the node passes the request on to the leader.
	if (raft->isLeader())
	{
		std::vector<std::string> lines = Utility::splitStringOn(data, ENTRY_DELIMITER_CHAR);
		if (lines.size() < 3)
		{
			return HTTPStatusCodes::clientError("Error: not enough lines.");
		}
		// Get crawlID and identifier of crawl job.
		std::vector<std::string> identifiers = Utility::splitStringOn(lines[0], FIELD_DELIMITER_CHAR);
		if (identifiers.size() < 2)
		{
			return HTTPStatusCodes::clientError("Error: oncorrect amount of identifiers.");
		}
		long long jobID = Utility::safeStoll(identifiers[1]);
		if (errno != 0)
		{
			return HTTPStatusCodes::clientError("Error: invalid jobID.");
		}
		if (jobID == timeLastCrawl)
		{
			int id = Utility::safeStoi(identifiers[0]);
			if (errno != 0)
			{
				return HTTPStatusCodes::clientError("Error: invalid crawlID.");
			}
			updateCrawlID(id);

			if (lines[1] != "")
			{
				std::vector<std::string> languages = Utility::splitStringOn(lines[1], FIELD_DELIMITER_CHAR);
				for (int i = 0; i < languages.size(); i += 2)
				{
					stats->languageCounter->Add({{"Node", stats->myIP}, {"Client", client}, {"Language", languages[i]}})
						.Increment(Utility::safeStoll(languages[i + 1]));
				}
			}

			// Get data after crawlID and pass it on to handleUploadRequest.
			return handleUploadJobRequest(request, client, std::vector<std::string>(lines.begin() + 2, lines.end()));
		}
		else
		{
			return HTTPStatusCodes::clientError("Crawl job was deprecated, a new job has already been issued.");
		}
	}
	return raft->passRequestToLeader(request, client, data);
}

void JobRequestHandler::updateCrawlID(int id)
{
	timeLastCrawl = -1;
	if (crawlID != id)
	{
		crawlID = id;
		database->setCrawlID(id);
	}
}

void JobRequestHandler::connectWithRetry(std::string ip, int port)
{
	errno = 0;
	database->connect(ip, port);
	int tries = 0;
	if (errno != 0)
	{
		while (tries < MAX_RETRIES)
		{
			usleep(pow(2, tries) * RETRY_SLEEP);
			database->connect(ip, port);
			if (errno == 0)
			{
				return;
			}
			tries++;
		}
		throw "Unable to connect to database.";
	}
	errno = 0;
}

std::string JobRequestHandler::getTopJobWithRetry()
{
	std::tuple<std::string, std::string, long long, long long> topJob = database->getTopJob();
	std::string jobid = std::get<0>(topJob);
	std::string url = std::get<1>(topJob);
	long long currTime = std::get<2>(topJob);
	long long timeout = std::get<3>(topJob);
	int tries = 0;
	if (errno != 0)
	{
		while (tries < MAX_RETRIES)
		{
			usleep(pow(2, tries) * RETRY_SLEEP);
			topJob = database->getTopJob();
			jobid = std::get<0>(topJob);
			url = std::get<1>(topJob);
			currTime = std::get<2>(topJob);
			timeout = std::get<3>(topJob);
			if (errno == 0)
			{
				return HTTPStatusCodes::success(
					std::string("Spider") + FIELD_DELIMITER_CHAR + jobid + FIELD_DELIMITER_CHAR + url +
					FIELD_DELIMITER_CHAR + std::to_string(currTime) + FIELD_DELIMITER_CHAR + std::to_string(timeout));
			}
			tries++;
		}
		return HTTPStatusCodes::serverError("Unable to get job from database.");
	}
	numberOfJobs -= 1;
	return HTTPStatusCodes::success(std::string("Spider") + FIELD_DELIMITER_CHAR + jobid + FIELD_DELIMITER_CHAR + url +
									FIELD_DELIMITER_CHAR + std::to_string(currTime) + FIELD_DELIMITER_CHAR +
									std::to_string(timeout));
}

bool JobRequestHandler::tryUploadJobWithRetry(std::string url, int priority, int retries, long long timeout)
{
	int tries = 0;
	database->uploadJob(url, priority, retries, timeout);
	if (errno != 0)
	{
		while (tries < MAX_RETRIES)
		{
			usleep(pow(2, tries) * RETRY_SLEEP);
			database->uploadJob(url, priority, retries, timeout);
			if (errno == 0)
			{
				return true;
			}
			tries++;
		}
		return false;
	}
	return true;
}

void JobRequestHandler::addFailedJobWithRetry(std::string id, long long currTime, long long priority, std::string url,
											  int retries, int reasonID, std::string reasonData)
{
	int tries = 0;
	database->addFailedJob(id, currTime, prioritym url, retries, reasonID, reasonData);
	if (errno != 0)
	{
		while (tries < MAX_RETRIES)
		{
			usleep(pow(2, tries) * RETRY_SLEEP);
			database->addFailedJob(id, currTime, prioritym url, retries, reasonID, reasonData);
			if (errno == 0)
			{
				return;
			}
			tries++;
		}
		return;
	}
	return;
}

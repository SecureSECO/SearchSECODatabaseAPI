/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "JobRequestHandler.h"
#include "Definitions.h"
#include "HTTPStatus.h"
#include "Utility.h"
#include "JobTypes.h"
#include <iostream>

using namespace jobTypes;

JobRequestHandler::JobRequestHandler(RAFTConsensus *raft, RequestHandler *requestHandler, DatabaseConnection *database,
									 Statistics *stats, std::string ip, int port)
{
	this->raft = raft;
	this->requestHandler = requestHandler;
	this->database = database;
	this->stats = stats;
	connectWithRetry(ip, port);
	database->getNumberOfJobs();
	crawlID = database->getCrawlID();
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
		if (database->getNumberOfJobs() >= MIN_AMOUNT_JOBS || (alreadyCrawling == true && database->getNumberOfJobs() >= 1))
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

std::string JobRequestHandler::handleUpdateJobRequest(std::string request, std::string client, std::string data)
{
	// Request should only be processed by leader.
	if (raft->isLeader())
	{
		std::vector<std::string> splitted = Utility::splitStringOn(data, FIELD_DELIMITER_CHAR);
		if (splitted.size() < 2)
		{
			return HTTPStatusCodes::clientError("Incorrect amount if arguments. Expected job id and time.");
		}
		std::string jobid = splitted[0];
		std::cout << "Updating job: " << jobid << std::endl;
		long long jobTime = Utility::safeStoll(splitted[1]);
		if (errno != 0)
		{
			return HTTPStatusCodes::clientError("Incorrect job time.");
		}

		Job job = getCurrentJobWithRetry(jobid);

		// If job was not present in currentjobs (or an error was thrown),
		if (job.jobid == "")
		{
			// a newer job was already given out and finished. Signal this to the worker.
			std::cout << "Received unknown job with ID: " << jobid << std::endl;

			return HTTPStatusCodes::clientError("Job not currently expected.");
		}

		// Check if a newer version of the job is currently busy.
		if (jobTime < job.time)
		{
			return HTTPStatusCodes::clientError("Job not currently expected.");
		}
		else if (jobTime > job.time)
		{
			std::cout << "Received job was newer than newest job. (" << std::to_string(jobTime) << " > "
					  << std::to_string(job.time) << ")" << std::endl
					  << "JobID: " << job.jobid << std::endl;
			return HTTPStatusCodes::clientError("Job not currently expected.");
		}

		// Update timeout timer in currentjobs table.
		long long newTime = addCurrentJobWithRetry(job);
		return HTTPStatusCodes::success(std::to_string(newTime));
	}
	// If you are not the leader, pass the request to the leader.
	return raft->passRequestToLeader(request, client, data);
}

std::string JobRequestHandler::handleFinishJobRequest(std::string request, std::string client, std::string data)
{
	// Request should only be processed by leader.
	if (raft->isLeader())
	{
		std::vector<std::string> splitted = Utility::splitStringOn(data, FIELD_DELIMITER_CHAR);
		if (splitted.size() < 4)
		{
			return HTTPStatusCodes::clientError("Incorrect amount of arguments.");
		}
		std::string jobid = splitted[0];
		std::cout << "Finishing job: " << jobid << std::endl;
		long long jobTime = Utility::safeStoll(splitted[1]);
		if (errno != 0)
		{
			return HTTPStatusCodes::clientError("Incorrect job time.");
		}
		int reasonID = Utility::safeStoi(splitted[2]);
		if (errno != 0)
		{
			return HTTPStatusCodes::clientError("Incorrect reason id.");
		}
		std::string reasonData = splitted[3];

		stats->jobCounter->Add({{"Node", stats->myIP}, {"Client", client}, {"Reason", std::to_string(reasonID)}}).Increment();

		Job job = getCurrentJobWithRetry(jobid);

		// If job was not present in currentjobs (or an error was thrown),
		if (job.jobid == "")
		{
			// a newer job was already given out and finished. Signal this to the worker.
			std::cout << "Received unknown job with ID: " << jobid << std::endl;

			return HTTPStatusCodes::clientError("Job not currently expected.");
		}

		// Check if a newer version of the job is currently busy.
		if (jobTime < job.time)
		{
			return HTTPStatusCodes::clientError("Job not currently expected.");
		}
		else if (jobTime > job.time)
		{
			std::cout << "Received job was newer than newest job. (" << std::to_string(jobTime) << " > "
					  << std::to_string(job.time) << ")" << std::endl
					  << "JobID: " << job.jobid << std::endl;
			return HTTPStatusCodes::clientError("Job not currently expected.");
		}

		// Check if the worker failed to complete the job.
		if (reasonID != 0)
		{
			addFailedJobWithRetry(FailedJob(job, reasonID, reasonData));

			if (errno != 0)
			{
				std::cout << "Could not store failed job: " << data << std::endl;
				return HTTPStatusCodes::serverError("Job could not be added to failed jobs list.");
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
		if (dataSecondSplit.size() < 3)
		{
			return HTTPStatusCodes::clientError("Incorrect amount of arguments.");
		}
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
		tryUploadJobWithRetry(urls[i], priorities[i], 0, timeouts[i]);
		if (errno != 0)
		{
			return HTTPStatusCodes::serverError("Unable to add job " + std::to_string(i) + " to database.");
		}
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
	Job topJob = database->getTopJob();
	int tries = 0;
	if (errno != 0)
	{
		while (tries < MAX_RETRIES)
		{
			usleep(pow(2, tries) * RETRY_SLEEP);
			topJob = database->getTopJob();
			if (errno == 0)
			{
				return HTTPStatusCodes::success(
					std::string("Spider") + FIELD_DELIMITER_CHAR + topJob.jobid + FIELD_DELIMITER_CHAR + topJob.url +
					FIELD_DELIMITER_CHAR + std::to_string(topJob.time) + FIELD_DELIMITER_CHAR + std::to_string(topJob.timeout));
			}
			tries++;
		}
		return HTTPStatusCodes::serverError("Unable to get job from database.");
	}
	return HTTPStatusCodes::success(std::string("Spider") + FIELD_DELIMITER_CHAR + topJob.jobid + FIELD_DELIMITER_CHAR + topJob.url +
									FIELD_DELIMITER_CHAR + std::to_string(topJob.time) + FIELD_DELIMITER_CHAR +
									std::to_string(topJob.timeout));
}

void JobRequestHandler::tryUploadJobWithRetry(std::string url, int priority, int retries, long long timeout)
{
	std::function<bool()> function = [url, priority, retries, timeout, this]() {
		this->database->uploadJob(url, priority, retries, timeout, true);
		return true;
	};
	Utility::queryWithRetry(function);
}

Job JobRequestHandler::getCurrentJobWithRetry(std::string jobid)
{
	std::function<Job()> function = [jobid, this]() { return this->database->getCurrentJob(jobid); };
	return Utility::queryWithRetry(function);
}

long long JobRequestHandler::addCurrentJobWithRetry(Job job)
{
	std::function<long long()> function = [job, this]() { return this->database->addCurrentJob(job); };
	return Utility::queryWithRetry(function);
}

void JobRequestHandler::addFailedJobWithRetry(FailedJob job)
{
	std::function<bool()> function = [job, this]() {
		this->database->addFailedJob(job);
		return true;
	};
	Utility::queryWithRetry(function);
}

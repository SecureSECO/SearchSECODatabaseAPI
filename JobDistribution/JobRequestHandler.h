/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include "DatabaseConnection.h"
#include "RAFTConsensus.h"
#include "Utility.h"
#include "Definitions.h"

#include <boost/shared_ptr.hpp>

#define MIN_AMOUNT_JOBS 500
#define MAX_RETRIES 3
#define CRAWL_TIMEOUT_SECONDS 150

class TcpConnection;

class JobRequestHandler
{
public:
	/// <summary>
	/// Constructor method.
	/// </summary>
	JobRequestHandler(RAFTConsensus* raft, RequestHandler* requestHandler, DatabaseConnection* database, std::string ip, int port);

	/// <summary>
	/// Handles request from new node to connect to the network.
	/// </summary>
	std::string handleConnectRequest(boost::shared_ptr<TcpConnection> connection, std::string request);

	/// <summary>
	/// Handles request to upload one or more jobs with their priorities.
	/// </summary>
	/// <param name="data">
	/// Consists of url and priority pairs, the url and priority are separated by a FIELD_DELIMITER_CHAR ('?') and the pairs by ENTRY_DELIMITER_CHAR ('\n').
	/// Data format is url1?priority1\nurl2?priority2\n...
	/// </param>
	/// <returns>
	/// Response to user whether the job(s) has/have been uploaded succesfully or not.
	/// </returns>
	std::string handleUploadJobRequest(std::string request, std::string data);

	/// <summary>
	/// Handles request to give the top job from the queue.
	/// </summary>
	/// <returns>
	/// Response is "Spider?url", where url is the url of the first job in the database
	/// if there are enough in the database or if a crawler is already working.
	/// Response is "Crawl?crawlId", where crawlId is the current crawlId
	/// if the number of jobs is not enough and there is no crawler working.
	/// Response is "NoJob" if there are no jobs and a crawler is already working.
	/// </returns>
	std::string handleGetJobRequest(std::string request, std::string data);

	/// <summary>
	/// Handles request to upload crawl data to the job queue.
	/// </summary>
	/// <param name="data">
	/// Data is almost the same as in handleUploadJobRequest, but now with an id in front.
	/// Data format is id\nurl1?priority1\nurl2?priority2\n...
	/// </param>
	/// <returns>
	/// Returns the result of handleUploadRequest.
	/// </returns>
	std::string handleCrawlDataRequest(std::string request, std::string data);

	/// <summary>
	/// Variables describing the number of jobs in the jobqueue, the current crawlId
	/// which is needed by the crawler to crawl a specific part of github
	/// and if there is currently a crawler working.
	/// </summary>
	int numberOfJobs;
	int crawlId;
	// bool alreadyCrawling = false;
	long long timeLastCrawl = -1;

	/// <summary>
	/// Updates the crawlId when crawl data is uploaded to the database.
	/// </summary>
	/// <param name="id">
	/// New crawlId.
	/// </param>
	void updateCrawlId(int id);
private:
	RAFTConsensus* raft;
	RequestHandler* requestHandler;
	DatabaseConnection* database;

	/// <summary>
	/// Tries to connect with database, if it fails it retries as many times as MAX_RETRIES.
	/// If it succeeds, it connects with the database and returns.
	/// If it still fails on the last retry, it throws an exception.
	/// </summary>
	void connectWithRetry(std::string ip, int port);

	/// <summary>
	/// Tries to get the top job from the database, if it fails it retries like above.
	/// If it succeeds, it returns the url of the top job.
	/// If it fails, it returns an error message.
	/// </summary>
	std::string getTopJobWithRetry();

	/// <summary>
	/// Tries to upload a job to the database, if it fails it retries like above.
	/// If it succeeds, it returns true.
	/// If it fails, it returns false.
	/// </summary>
	bool tryUploadJobWithRetry(std::string url, int priority);
};

/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include "DatabaseConnection.h"
#include "RAFTConsensus.h"

#include <mutex>
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
	JobRequestHandler(RAFTConsensus *raft, RequestHandler *requestHandler, DatabaseConnection *database, Statistics *stats,
					std::string ip, int port);

	/// <summary>
	/// Handles request from new node to connect to the network.
	/// </summary>
	std::string handleConnectRequest(boost::shared_ptr<TcpConnection> connection, std::string request);

	/// <summary>
	/// Handles request for the ip adresses in the network.
	/// </summary>
	std::string handleGetIPsRequest(std::string request, std::string client, std::string data);

	/// <summary>
	/// Handles request to upload one or more jobs with their priorities.
	/// </summary>
	/// <param name="data">
	/// Consists of url and priority pairs, the url and priority are separated by the FIELD_DELIMITER_CHAR ('?') and
	/// the pairs by the ENTRY_DELIMITER_CHAR ('\n').
	/// Data format is "url1?priority1'\n'url2?priority2'\n'..."
	/// </param>
	/// <returns>
	/// Response to user whether the job(s) has/have been uploaded succesfully or not.
	/// </returns>
	std::string handleUploadJobRequest(std::string request, std::string client, std::string data);

	/// <summary>
	/// Handles request to upload one or more jobs with their priorities.
	/// </summary>
	/// <param name="data">
	/// Consists of url and priority pairs, the url and priority are separated by the FIELD_DELIMITER_CHAR ('?') and
	/// the pairs by the ENTRY_DELIMITER_CHAR ('\n').
	/// Data format is "url1?priority1'\n'url2?priority2'\n'..."
	/// </param>
	/// <returns>
	/// Response to user whether the job(s) has/have been uploaded succesfully or not.
	/// </returns>
	std::string handleUploadJobRequest(std::string request, std::string client, std::vector<std::string> data);

	/// <summary>
	/// Handles request to give the top job from the queue.
	/// </summary>
	/// <returns>
	/// Response is "Spider?url", where url is the url of the first job in the database
	/// if there are enough in the database or if a crawler is already working.
	/// Response is "Crawl?crawlID", where crawlID is the current crawlID
	/// if the number of jobs is not enough and there is no crawler working.
	/// Response is "NoJob" if there are no jobs and a crawler is already working.
	/// </returns>
	std::string handleGetJobRequest(std::string request, std::string client, std::string data);

	/// <summary>
	/// 
	/// </summary>
	std::string handleUpdateJobRequest(std::string request, std::string client, std::string data);

	/// <summary>
	/// Handles request to indicate the worker is finished with a job, successfully or not.
	/// </summary>
	/// <returns>
	/// Response is "Job not currently expected." if a newer version of the job was
	/// given out, or if the job is not known to have been given out. Reponse is
	/// "Job finished successfully" on success and "Job failed successfully" on 
	/// a client-side failure, which was correctly handled and uploaded.
	/// For a client-side failure, when the database fails to upload the current job
	/// to the failedjobs table, "Job could not be added to failed jobs list." is returned.
	/// </returns>
	std::string handleFinishJobRequest(std::string request, std::string client, std::string data);

	/// <summary>
	/// Handles request to upload crawl data to the job queue.
	/// </summary>
	/// <param name="data">
	/// Data is almost the same as in handleUploadJobRequest, but now with an ID in front.
	/// Data format is: "id'\n'url1?priority1'\n'url2?priority2'\n'..."
	/// </param>
	/// <returns>
	/// Returns the result of handleUploadRequest.
	/// </returns>
	std::string handleCrawlDataRequest(std::string request, std::string client, std::string data);

	/// <summary>
	/// Variables describing the current crawlID
	/// which is needed by the crawler to crawl a specific part of GitHub
	/// and if there is currently a crawler working.
	/// </summary>	
	int crawlID;
	long long timeLastCrawl = -1;

	/// <summary>
	/// Updates the crawlID when crawl data is uploaded to the database.
	/// </summary>
	/// <param name="id">
	/// New crawlID.
	/// </param>
	void updateCrawlID(int id);

	DatabaseConnection *getDatabaseConnection()
	{
		return database;
	}

private:
	RAFTConsensus *raft;
	RequestHandler *requestHandler;
	DatabaseConnection *database;
	Statistics *stats;
	std::mutex jobmtx;

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
	void tryUploadJobWithRetry(std::string url, int priority, int retries, long long timeout);

	/// <summary>
	/// Attempts to retrieve a match from the currentjobs table with a matching jobid.
	/// </summary>
	/// <param name="jobid">The jobid to match on.</param>
	/// <returns>
	/// A Job object containing information on the current job. 
	/// Returns a nullptr if no such job was found.
	/// </returns>
	Job getCurrentJobWithRetry(std::string jobid);

	/// <summary>
	/// Adds the given job to the currentjobs table.
	/// </summary>
	long long addCurrentJobWithRetry(Job job);

	/// <summary>
	/// Adds a job with the given parameters to the failedjobs table with retry.
	/// </summary>
	void addFailedJobWithRetry(FailedJob job);
};

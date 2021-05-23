/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include "DatabaseConnection.h"
#include "RAFTConsensus.h"
#include "Utility.h"

#include <boost/shared_ptr.hpp>

#define MIN_AMOUNT_JOBS 500

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
        /// Handles request to upload one or more jobs with their priorities..
        /// </summary>
	std::string handleUploadJobRequest(std::string request, std::string data);

	/// <summary>
        /// Handles request to give the top job from the queue.
        /// </summary>
	std::string handleGetJobRequest(std::string request, std::string data);

	/// <summary>
        /// Handles request to upload crawl data to the job queue.
        /// </summary>
	std::string handleCrawlDataRequest(std::string request, std::string data);

	/// <summary>
	/// Variables describing the number of jobs in the jobqueue, the current crawlId
	/// and if there is currently a crawler working.
	/// </summary>
	int numberOfJobs;
	int crawlId;
	bool alreadyCrawling = false;

	void updateCrawlId(int id);
private:

	RAFTConsensus* raft;
	RequestHandler* requestHandler;
	DatabaseConnection* database;

};

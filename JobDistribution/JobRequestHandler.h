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
	/// Data format is url1?priority1\nurl2?priority2\n...
        /// </summary>
	std::string handleUploadJobRequest(std::string request, std::string data);

	/// <summary>
        /// Handles request to give the top job from the queue.
        /// </summary>
	std::string handleGetJobRequest(std::string request, std::string data);

	/// <summary>
        /// Handles request to upload crawl data to the job queue.
	/// Data format is id\nurl1?priority1\nurl2?priority2\n...
        /// </summary>
	std::string handleCrawlDataRequest(std::string request, std::string data);

private:

	RAFTConsensus* raft;
	RequestHandler* requestHandler;
	DatabaseConnection* database;

};

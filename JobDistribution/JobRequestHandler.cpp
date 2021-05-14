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

std::string JobRequestHandler::handleUploadJobRequest(std::string request, std::string url)
{
    if (raft->isLeader())
    {
        database->uploadJob(url);
        if (errno == 0)
        {
            return "Your job has been succesfully added to the queue.";
        }
        else
        {
            return "An error has occurred while adding your job to the queue.";
        }
    }
    return raft->passRequestToLeader(request, url);
}


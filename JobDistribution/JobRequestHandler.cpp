/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "JobRequestHandler.h"

#include <iostream>

JobRequestHandler::JobRequestHandler(RAFTConsensus* raft, RequestHandler* requestHandler) 
{
    this->raft = raft;
    this->requestHandler = requestHandler;
}

std::string JobRequestHandler::handleConnectRequest(boost::shared_ptr<TcpConnection> connection)
{
    return raft->connectNewNode(connection);
}

std::string JobRequestHandler::addJob(std::string request, std::string data)
{
    if (raft->isLeader()) 
    {
        // TODO: Handle localy
        std::cout << "REQUEST " << request << ". DATA: " << data << "\n";
    }
    else 
    {
        raft->passRequestToLeader(request, data);
        std::cout << "Sending request to leader.\n";
    }
    return "Job added successfully";
}
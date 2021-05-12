/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/
#pragma once

#define LEADER_IPS { {"131.211.31.153", "8003"}}

#include <vector>

#include "Networking.h"

#include <boost/shared_ptr.hpp>

class TcpConnection;
class RequestHandler;

class RAFTConsensus 
{
public:
	void start(RequestHandler* requestHandler, bool assumeLeader = false);

	bool isLeader() { return leader; };

	std::string passRequestToLeader(std::string requestType, std::string request);

	std::string connectNewNode(boost::shared_ptr<TcpConnection> connection);
private:
	void connectToLeader();

	void listenForHeartbeat();
	void handleHeartbeat(std::string heartbeat);

	void heartbeatSender();
	void listenForRequests(boost::shared_ptr<TcpConnection> connection);

    bool leader;

	// Non-leader variables.
	NetworkHandler* networkhandler;
	std::string leaderIp, leaderPort;

	// Leader variables.
	std::vector<boost::shared_ptr<TcpConnection>>* others;
	RequestHandler* requestHandler;
};

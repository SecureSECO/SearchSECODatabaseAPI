/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/
#pragma once

#define LEADER_IPS { {"127.0.0.1", "8002"}}

#include <vector>
#include <boost/shared_ptr.hpp>
#include <mutex>

#include "Networking.h"


class TcpConnection;
class RequestHandler;

class RAFTConsensus 
{
public:
	void start(RequestHandler* requestHandler, bool assumeLeader = false);

	bool isLeader() { return leader; };

	std::string passRequestToLeader(std::string requestType, std::string request);

	std::string connectNewNode(boost::shared_ptr<TcpConnection> connection, std::string request);
private:
	void connectToLeader();

	void listenForHeartbeat();
	void handleHeartbeat(std::string heartbeat);
	void dropConnection(int i);

	void heartbeatSender();
	std::string getHeartbeat();
	void listenForRequests(boost::shared_ptr<TcpConnection> connection);

	std::string connectionToString(boost::shared_ptr<TcpConnection> connection, std::string port);

    bool leader;
	std::mutex mtx;

	// Non-leader variables.
	NetworkHandler* networkhandler;
	std::string leaderIp, leaderPort;
	std::vector<std::pair<std::string, std::string>> nonLeaderNodes;


	// Leader variables.
	std::vector<std::pair<boost::shared_ptr<TcpConnection>, std::string>>* others;
	RequestHandler* requestHandler;
	std::string nodeConnectionChange = "";
};

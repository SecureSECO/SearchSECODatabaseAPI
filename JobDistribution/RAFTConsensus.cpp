/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "RAFTConsensus.h"
#include "Networking.h"
#include "ConnectionHandler.h"
#include "Utility.h"

#include <vector>
#include <map>
#include <thread>
#include <iostream>

#define RESPONSE_OK "ok"

void RAFTConsensus::start(RequestHandler* requestHandler, bool assumeLeader) 
{
	others = new std::vector<boost::shared_ptr<TcpConnection>>();
	leader = true;
	this->requestHandler = requestHandler;
	if (!assumeLeader) 
	{
		connectToLeader();
	}

	if (leader) 
	{
		// We are just going to use the request handler for the incomming requests.
		// TODO: Start heartbeat.
	}
	else 
	{
		// TODO: Listen for heartbeat.

	}
}

void RAFTConsensus::connectToLeader() 
{
	// Loop through all set IP's where we expect the leader to be
	std::vector<std::pair<std::string, std::string>> ips = LEADER_IPS;
	for (auto const& x : ips)
	{
		try 
		{
			// Try to connect to the set IP.
			std::string ip = x.first;
			std::string port = x.second;
			std::string response = "";
			while (response != RESPONSE_OK) 
			{
				networkhandler = NetworkHandler::createHandler();
				// If the IP + port are not open, this will throw an exception sending us to the catch.
				networkhandler->openConnection(ip, port);
				// Send a connect request. TODO: formaly document what requet we use for this.
				networkhandler->sendData("conn0\n\n");
				response = networkhandler->receiveData();
				std::vector<std::string> receivedLeader = Utility::splitStringOn(response, '?');
				if (receivedLeader[0] != RESPONSE_OK) 
				{
					// If we get something which is not an ok, we will assume that it has send back
					// the true leader.
					if(receivedLeader.size() != 2) 
					{
						throw "Incorrect response from connect request";
					}
					ip = receivedLeader[0];
					port = receivedLeader[1];
					delete networkhandler;
				}
				else 
				{
					// TODO: Use the data that the leader send back as initial data.
				}
			}
			// If we get through the while loop without throwing an exception, then we have found the leader.
			leaderIp = ip;
			leaderPort = port;
			leader = false;
			break;
		}
		catch (std::exception const& ex) 
		{
			std::cout << ex.what();
			delete networkhandler;
			continue;
		}
	}
}

void RAFTConsensus::listenForHeartbeat() 
{
	while (true) 
	{
		try 
		{
			std::string data = networkhandler->receiveData();
			handleHeartbeat(data);
		}
		catch(std::exception const& ex) 
		{
			// TODO: if receiving data fails, then the host is presumably dead,
			// so we then want to choose a new leader instead of stopping.
			break;
		}
	}
}

std::string RAFTConsensus::connectNewNode(boost::shared_ptr<TcpConnection> connection) 
{
	if (leader) 
	{
		others->push_back(connection);
		// TODO: look into what initial data we need to send.
		// TODO: Start listning to requests that are being send.
		new std::thread(&RAFTConsensus::listenForRequests, this, connection);
		return std::string(RESPONSE_OK) + "?Initial data\n";
	}
	return leaderIp + "?" + leaderPort;
}

void RAFTConsensus::handleHeartbeat(std::string heartbeat) 
{
	// TODO: implement this function properly;
	std::cout << "Heartbeat received: "  << heartbeat;
}

std::string RAFTConsensus::passRequestToLeader(std::string requestType, std::string request) 
{
	networkhandler->sendData(requestType + std::to_string(request.length()) + "\n" + request);
	return networkhandler->receiveData();
}

void RAFTConsensus::heartbeatSender()
{

}

void RAFTConsensus::listenForRequests(boost::shared_ptr<TcpConnection> connection) 
{
	try 
	{
		while(true) 
		{
			connection->start(requestHandler);
		}
	}
	catch (std::exception const& ex) 
	{
		std::cout << ex.what();
	}
}
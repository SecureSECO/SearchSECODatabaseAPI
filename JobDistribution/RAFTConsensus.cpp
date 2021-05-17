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
#include <algorithm>

#define RESPONSE_OK "ok"
#define HEARTBEAT_TIME 1000000
#define LEADER_DROPOUT_WAIT_TIME 1000000

void RAFTConsensus::start(RequestHandler* requestHandler, bool assumeLeader) 
{
	others = new std::vector<std::pair<boost::shared_ptr<TcpConnection>, std::string>>();
	leader = true;
	this->requestHandler = requestHandler;
	if (!assumeLeader) 
	{
		connectToLeader(LEADER_IPS);
	}

	if (leader) 
	{
		new std::thread(&RAFTConsensus::heartbeatSender, this);
	}
	else 
	{
		new std::thread(&RAFTConsensus::listenForHeartbeat, this);
	}
}

void RAFTConsensus::connectToLeader(std::vector<std::pair<std::string, std::string>> ips) 
{
	// Loop through all set IP's where we expect the leader to be
	for (auto const& x : ips)
	{
		try 
		{
			// Try to connect to the set IP.
			std::string ip = x.first;
			std::string port = x.second;
			std::string response = "";
			while (response.substr(0, std::string(RESPONSE_OK).size()) != RESPONSE_OK) 
			{
				tryConnectingWithIp(ip, port, response);
			}
			// If we get through the while loop without throwing an exception, then we have found the leader.
			leaderIp = ip;
			leaderPort = port;
			leader = false;
			std::cout << "Found leader " + ip + " " + port + "\n";
			break;
		}
		catch (std::exception const& ex) 
		{
			std::cout << ex.what() << "\n";
			delete networkhandler;
			continue;
		}
	}
}

void RAFTConsensus::handleInitialData(std::vector<std::string> initialData)
{
	if (initialData.size() < 3)
	{
		throw std::runtime_error("Incorrect initial data.");
	}
	myIp = initialData[1];
	myPort = initialData[2];
	// We check i + 1 instead if just i, because we need 2 values every time.
	for (int i = 3; i + 1 < initialData.size(); i += 2) 
	{
		nonLeaderNodes.push_back(std::pair<std::string, std::string>(initialData[i], initialData[i + 1]));
	}
}

void RAFTConsensus::tryConnectingWithIp(std::string &ip, std::string &port, std::string &response)
{
	networkhandler = NetworkHandler::createHandler();
	// If the IP + port are not open, this will throw an exception sending us to the catch.
	networkhandler->openConnection(ip, port);
	// Send a connect request. TODO: formaly document what request we use for this.
	networkhandler->sendData(
		"conn" + std::to_string(1 + std::to_string(PORT).length()) + "\n" + std::to_string(PORT) + "\n");
	response = networkhandler->receiveData();
	std::vector<std::string> receivedLeader = Utility::splitStringOn(response, '?');
	if (receivedLeader[0] != RESPONSE_OK) 
	{
		// If we get something which is not an ok, we will assume that it has send back the true leader.
		if(receivedLeader.size() != 2) 
		{
			throw std::runtime_error("Incorrect response from connect request. Size was " + std::to_string(receivedLeader.size()));
		}
		ip = receivedLeader[0];
		port = receivedLeader[1];
		delete networkhandler;
	}
	else 
	{
		handleInitialData(receivedLeader);
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
			auto newLeader = nonLeaderNodes[0];
			nonLeaderNodes.clear();
			if (newLeader.first == myIp && newLeader.second == myPort)
			{ 
				// We are next in the list, so we are going to assume leadership.
				delete others;
				start(requestHandler, true);
				return;
			}
			usleep(LEADER_DROPOUT_WAIT_TIME);
			std::cout << "Attempt connection with new leader.";
			connectToLeader({newLeader});
			if(leader) 
			{
				throw std::runtime_error("Could not connect with the new leader.");
			}
		}
	}
}

std::string RAFTConsensus::connectNewNode(boost::shared_ptr<TcpConnection> connection, std::string request) 
{
	if (leader) 
	{

		request = request.substr(0, request.length() - 1);

		mtx.lock();
		std::string initialData = "";
		for (auto con : *others) 
		{
			initialData += "?" + connectionToString(con.first, con.second);
		}

		others->push_back(std::pair<boost::shared_ptr<TcpConnection>, std::string>(connection, request));
		if(nodeConnectionChange != "") 
		{
			nodeConnectionChange += "?";
		}
		nodeConnectionChange += "A?" + connectionToString(connection, request);

		mtx.unlock();
		// TODO: look into what initial data we need to send.
		new std::thread(&RAFTConsensus::listenForRequests, this, connection);

		std::string connectingIp = "?" + connectionToString(connection, request);

		return std::string(RESPONSE_OK) + connectingIp + initialData + "\n";
	}
	return leaderIp + "?" + leaderPort + "\n";
}

void RAFTConsensus::handleHeartbeat(std::string heartbeat) 
{
	std::cout << "Heartbeat received: "  << heartbeat << "\n";
	std::vector<std::string> hbSplitted = Utility::splitStringOn(heartbeat, '?');
	for (int i = 0; i < hbSplitted.size(); i += 3) 
	{
		std::pair<std::string, std::string> pairReceived = 
			std::pair<std::string, std::string>(hbSplitted[i+1], hbSplitted[i+2]);
		if (hbSplitted[i] == "A") 
		{
			bool found = false;
			for (auto node : nonLeaderNodes)
			{
				if (pairReceived == node) {
					found = true;
					break;
				}
			}
			if (!found) 
			{
				nonLeaderNodes.push_back(pairReceived);
			}
		}
		else if (hbSplitted[i] == "R")
		{
			nonLeaderNodes.erase(
				std::remove(nonLeaderNodes.begin(), nonLeaderNodes.end(), 
				pairReceived), 
				nonLeaderNodes.end());
		}
		else 
		{
			std::cout << "Incorrect heartbeat\n";
		}
	}
}

std::string RAFTConsensus::passRequestToLeader(std::string requestType, std::string request) 
{
	networkhandler->sendData(requestType + std::to_string(request.length()) + "\n" + request);
	return networkhandler->receiveData();
}

void RAFTConsensus::heartbeatSender()
{
	while (true) 
	{
		usleep(HEARTBEAT_TIME);
		mtx.lock();
		std::string data = getHeartbeat();
		mtx.unlock();
		boost::system::error_code error;
		// Going from back to front, so we don't have to worry as much about connections
		// being added or removed while we are in this loop.
		
		mtx.lock();
		for (int i = others->size() - 1; i >= 0; i--) 
		{
			try 
			{
				boost::shared_ptr<TcpConnection> connection = others->at(i).first;
				
				boost::asio::write(connection->socket(), boost::asio::buffer(data), error);
				if (error)
				{
					std::cout << "Error "<< error << " sending data: Connection dropped.\n";
					dropConnection(i);
				}
			}
			catch(std::exception const& ex)
			{
				// If an error occured, we will assume the connection has dropped.
				std::cout << "Connection dropped.\n";
				dropConnection(i);
				
			}
		}
		mtx.unlock();
	}
}

void RAFTConsensus::dropConnection(int i) 
{
	std::pair<boost::shared_ptr<TcpConnection>, std::string> c = others->at(i);
	if(nodeConnectionChange != "") 
	{
		nodeConnectionChange += "?";
	}
	nodeConnectionChange += "R?" + connectionToString(c.first, c.second);

	(*others)[i] = (*others)[others->size()-1];
	others->pop_back();
}

std::string RAFTConsensus::getHeartbeat()
{
	std::string hb = nodeConnectionChange + "\n";
	std::cout << "Sending heartbeat: " + hb;
	nodeConnectionChange = "";
	return hb;
}

void RAFTConsensus::listenForRequests(boost::shared_ptr<TcpConnection> connection) 
{
	try 
	{
		while(true) 
		{
			connection->start(requestHandler, connection);
		}
	}
	catch (std::exception const& ex) 
	{
		std::cout << ex.what();
	}
}

std::string RAFTConsensus::connectionToString(boost::shared_ptr<TcpConnection> c, std::string port)
{
	return c->socket().remote_endpoint().address().to_string()
		+ "?" + port;
}
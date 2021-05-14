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
		new std::thread(&RAFTConsensus::heartbeatSender, this);
	}
	else 
	{
		// TODO: Listen for heartbeat.
		new std::thread(&RAFTConsensus::listenForHeartbeat, this);
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
				networkhandler->sendData("conn1\n\n");
				response = networkhandler->receiveData();
				std::vector<std::string> receivedLeader = Utility::splitStringOn(response, '?');
				if (receivedLeader[0] != RESPONSE_OK) 
				{
					// If we get something which is not an ok, we will assume that it has send back
					// the true leader.
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
					// TODO: Use the data that the leader send back as initial data.
					for (int i = 1; i < receivedLeader.size(); i += 2) 
					{
						// TODO: handle incorrect size.
						std::cout << "Node: " << receivedLeader[i] << " " << receivedLeader[i + 1] << "\n";
						nonLeaderNodes.push_back(std::pair<std::string, std::string>(receivedLeader[i], receivedLeader[i + 1]));
					}
					break;
				}
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

		mtx.lock();
		std::string initialData = "";
		for (auto con : *others) 
		{
			initialData += "?" + connectionToString(con);
		}

		others->push_back(connection);
		if(nodeConnectionChange != "") 
		{
			nodeConnectionChange += "?";
		}
		nodeConnectionChange += "A?" + connectionToString(connection);

		mtx.unlock();
		// TODO: look into what initial data we need to send.
		new std::thread(&RAFTConsensus::listenForRequests, this, connection);
		return std::string(RESPONSE_OK) + initialData + "\n";
	}
	return leaderIp + "?" + leaderPort;
}

void RAFTConsensus::handleHeartbeat(std::string heartbeat) 
{
	// TODO: implement this function properly.
	std::cout << "Heartbeat received: "  << heartbeat << "\n";
	std::vector<std::string> hbSplitted = Utility::splitStringOn(heartbeat, '?');
	std::cout << "\nSplitted size" <<hbSplitted.size() << "\n";
	for (int i = 0; i < hbSplitted.size(); i += 3) 
	{
		if (hbSplitted[i] == "A") 
		{
			std::cout << "Adding: " << hbSplitted[i+1] << " " << hbSplitted[i+2];
			nonLeaderNodes.push_back(std::pair<std::string, std::string>(hbSplitted[i+1], hbSplitted[i+2]));
		}
		else if (hbSplitted[i] == "R")
		{
			nonLeaderNodes.erase(
				std::remove(nonLeaderNodes.begin(), nonLeaderNodes.end(), 
				std::pair<std::string, std::string>(hbSplitted[i+1], hbSplitted[i+2])), 
				nonLeaderNodes.end());
		}
		else 
		{
			std::cout << "Incorrect heartbeat\n";
		}
	}
	std::cout << "New list of non leader nodes: \n";
	for (auto x : nonLeaderNodes) 
	{
		std::cout << "\t" << x.first << " " << x.second << "\n";
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
				boost::shared_ptr<TcpConnection> connection = others->at(i);
				
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
	boost::shared_ptr<TcpConnection> c = others->at(i);
	if(nodeConnectionChange != "") 
	{
		nodeConnectionChange += "?";
	}
	nodeConnectionChange += "R?" + connectionToString(c);

	(*others)[i] = (*others)[others->size()-1];
	others->pop_back();
}

std::string RAFTConsensus::getHeartbeat()
{
	// TODO: More heartbeat data.
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

std::string RAFTConsensus::connectionToString(boost::shared_ptr<TcpConnection> c)
{
	return c->socket().remote_endpoint().address().to_string()
		+ "?" + std::to_string(c->socket().remote_endpoint().port());
}
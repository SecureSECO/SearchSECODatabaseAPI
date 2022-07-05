/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "Definitions.h"
#include "RAFTConsensus.h"
#include "Networking.h"
#include "ConnectionHandler.h"
#include "JobRequestHandler.h"
#include "Utility.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <thread>
#include <vector>

RAFTConsensus::~RAFTConsensus() 
{
	if (started) 
	{
		stop = true;
		// Sleep so we know for sure the thread has stopped before we delete everything.
		usleep(HEARTBEAT_TIME + HEARTBEAT_TIME / 5);
	}
}

std::vector<std::pair<std::string, std::string>> RAFTConsensus::getIps(std::string file) 
{
	std::ifstream fileHandler;
	fileHandler.open(file);
	
	if (!fileHandler.is_open()) 
	{
		std::cout << "Unable to open .env file." << std::endl;
		return {};
	}
	std::string line;
	int total = 0;
	std::vector<std::pair<std::string, std::string>> output = {};
	while (std::getline(fileHandler, line)) 
	{
		auto lineSplitted = Utility::splitStringOn(line, '=');
		if (lineSplitted.size() >= 2 && lineSplitted[0] == "SEEDS") 
		{
			auto ipsSplitted = Utility::splitStringOn(lineSplitted[1], ',');			
			for (std::string ip : ipsSplitted) 
			{
				output.push_back(std::pair<std::string, std::string>(ip, std::to_string(PORT)));
			}
			total++;
		}
		else if (lineSplitted.size() >= 2 && lineSplitted[0] == "IP")
		{
			myIp = lineSplitted[1];
			myPort = std::to_string(PORT);
			total++;
		}
	}
	if (total < 2)
	{
		std::cout << "No SEEDS or IP entry found in .env file or SEEDS or IP entry was empty." << std::endl;
	}
	return output;
}

std::vector<std::string> RAFTConsensus::getCurrentIPs()
{
	std::vector<std::string> result = std::vector<std::string>();
	for (auto ip : *others)
	{
		result.push_back(connectionToString(ip));
	}
	result.push_back(myIp + FIELD_DELIMITER_CHAR + myPort);
	return result;
}

void RAFTConsensus::start(RequestHandler* requestHandler, 
	std::vector<std::pair<std::string, std::string>> ips, 
	bool assumeLeader) 
{
	started = true;
	others = new std::vector<Connection>();
	leader = true;
	this->requestHandler = requestHandler;
	if (!assumeLeader) 
	{
		connectToLeader(ips);
	}

	if (leader) 
	{
		new std::thread(&RAFTConsensus::heartbeatSender, this);
		new std::thread(&DatabaseConnection::updateCurrentJobs, requestHandler->getJobRequestHandler()->getDatabaseConnection());
	}
	else 
	{
		new std::thread(&RAFTConsensus::listenForHeartbeat, this);
	}
}

void RAFTConsensus::connectToLeader(std::vector<std::pair<std::string, std::string>> ips) 
{
	// Loop through all set IP's where we expect the leader to be.
	for (auto const& ipPort : ips)
	{
		try 
		{
			// Try to connect to the set IP.
			std::string ip = ipPort.first;
			std::string port = ipPort.second;
			std::string response = "";
			while (response.substr(0, std::string(RESPONSE_OK).size()) != RESPONSE_OK) 
			{
				tryConnectingWithIp(ip, port, response);
			}
			// If we get through the while loop without throwing an exception, then we have found the leader.
			leaderIp = ip;
			leaderPort = port;
			leader = false;
			break;
		}
		catch (std::exception const& e) 
		{
			std::cout << e.what() << std::endl;
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

	std::string entryDelimiter(1, ENTRY_DELIMITER_CHAR);
	networkhandler->sendData("conn" + std::string(1, FIELD_DELIMITER_CHAR) + "node" + myIp + FIELD_DELIMITER_CHAR +
							 std::to_string(2 + std::to_string(PORT).length() + myIp.length()) + entryDelimiter + myIp +
							 FIELD_DELIMITER_CHAR + std::to_string(PORT) + entryDelimiter);

	response = networkhandler->receiveData();
	std::vector<std::string> receivedLeader = Utility::splitStringOn(response, FIELD_DELIMITER_CHAR);
	if (receivedLeader[0] != RESPONSE_OK)
	{
		// If we get something which is not an OK, we will assume that it has send back the true leader.
		if (receivedLeader.size() != 2)
		{
			throw std::runtime_error("Incorrect response from connect request. Size was " +
									 std::to_string(receivedLeader.size()));
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
	while (!stop) 
	{
		std::string data;
		try 
		{
			data = networkhandler->receiveData();			
		}
		catch(std::exception const& ex) 
		{
			auto newLeader = nonLeaderNodes[0];
			nonLeaderNodes.clear();
			if (newLeader.first == myIp && newLeader.second == myPort)
			{ 
				std::cout << "Old leader dropped, we are the next in line, so we are now the leader." << std::endl;
				// We are next in the list, so we are going to assume leadership.
				delete others;
				start(requestHandler, {}, true);
				return;
			}
			usleep(LEADER_DROPOUT_WAIT_TIME);
			std::cout << "Attempt connection with new leader." << std::endl;
			connectToLeader({newLeader});
			if(leader) 
			{
				throw std::runtime_error("Could not connect with the new leader.");
			}
		}
		handleHeartbeat(data);
	}
}

std::string RAFTConsensus::connectNewNode(boost::shared_ptr<TcpConnection> connection, std::string request) 
{
	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);
	std::string entryDelimiter(1, ENTRY_DELIMITER_CHAR);
	if (leader) 
	{
		request = request.substr(0, request.length() - 1);

		std::vector<std::string> splitted = Utility::splitStringOn(request, FIELD_DELIMITER_CHAR);

		mtx.lock();
		std::string initialData = "";
		for (auto con : *others) 
		{
			initialData += fieldDelimiter + connectionToString(con);
		}
		Connection conn = Connection(connection, splitted[0], splitted[1]);
		others->push_back(conn);
		if(nodeConnectionChange != "") 
		{
			nodeConnectionChange += fieldDelimiter;
		}
		nodeConnectionChange += "A" + fieldDelimiter + connectionToString(conn);

		mtx.unlock();
		new std::thread(&RAFTConsensus::listenForRequests, this, connection);

		std::string connectingIp = fieldDelimiter + connectionToString(conn);

		return std::string(RESPONSE_OK) + connectingIp + initialData + entryDelimiter;
	}
	return leaderIp + fieldDelimiter + leaderPort + entryDelimiter;
}

void RAFTConsensus::handleHeartbeat(std::string heartbeat) 
{
	std::vector<std::string> hbSplitted = Utility::splitStringOn(heartbeat, FIELD_DELIMITER_CHAR);

	if (hbSplitted.size() == 0) 
	{
		return;
	}
	int crawlid = Utility::safeStoi(hbSplitted[0]);
	if (errno == 0) 
	{
		requestHandler->getJobRequestHandler()->crawlID = crawlid;
	}
	else 
	{
		std::cout << "Error parsing crawlid from heartbeat.\n";
	}
	for (int i = 1; i < hbSplitted.size(); i += 3) 
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
			std::cout << "Incorrect heartbeat" << std::endl;
		}
	}
}

std::string RAFTConsensus::passRequestToLeader(std::string requestType, std::string client, std::string request)
{
	std::string received = "";

	try
	{
		std::string entryDelimiter(1, ENTRY_DELIMITER_CHAR);
		NetworkHandler *networking = NetworkHandler::createHandler();

		networking->openConnection(leaderIp, leaderPort);
		networking->sendData(requestType + FIELD_DELIMITER_CHAR + client + FIELD_DELIMITER_CHAR +
							 std::to_string(request.length()) + entryDelimiter + request);

		received = networking->receiveData(false);
		delete networking;
	}
	catch (std::exception const &ex)
	{
		std::cout << "Receive data gave an error" << std::endl;
	}
	return received;
}

void RAFTConsensus::heartbeatSender()
{
	while (!stop) 
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
				boost::shared_ptr<TcpConnection> connection = others->at(i).connection;
				
				connection->sendData(data, error);
				if (error)
				{
					std::cout << "Error "<< error << " sending data: Connection dropped." << std::endl;
					dropConnection(i);
				}
			}
			catch(std::exception const& ex)
			{
				// If an error occured, we assume the connection has dropped.
				std::cout << "Connection dropped." << std::endl;
				dropConnection(i);
				
			}
		}
		mtx.unlock();
	}
}

void RAFTConsensus::dropConnection(int i) 
{
	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);

	Connection c = others->at(i);
	if(nodeConnectionChange != "") 
	{
		nodeConnectionChange += fieldDelimiter;
	}
	nodeConnectionChange += "R" + fieldDelimiter + connectionToString(c);

	(*others)[i] = (*others)[others->size()-1];
	others->pop_back();
}

std::string RAFTConsensus::getHeartbeat()
{
	std::string entryDelimiter(1, ENTRY_DELIMITER_CHAR);
	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);
	JobRequestHandler* jrh = requestHandler->getJobRequestHandler();

	std::string hb = std::to_string(jrh->crawlID) + 
		fieldDelimiter + nodeConnectionChange + entryDelimiter;
	nodeConnectionChange = "";
	return hb;
}

void RAFTConsensus::listenForRequests(boost::shared_ptr<TcpConnection> connection) 
{
	try 
	{
		while(!stop) 
		{
			connection->start(requestHandler, connection, stats);
		}
	}
	catch (std::exception const& e) 
	{
		std::cout << e.what();
	}
}

std::string RAFTConsensus::connectionToString(Connection connection)
{
	std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);
	return connection.ip + fieldDelimiter + connection.port;
}


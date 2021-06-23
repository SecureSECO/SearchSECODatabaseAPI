/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/
#pragma once


#include <vector>
#include <boost/shared_ptr.hpp>
#include <mutex>

#include "Networking.h"

#define RESPONSE_OK "ok"
#define HEARTBEAT_TIME 1000000
#define LEADER_DROPOUT_WAIT_TIME 1000000


class TcpConnection;
class RequestHandler;

class RAFTConsensus
{
public:
	~RAFTConsensus();

	/// <summary>
	/// Starts RAFT. Will try to connect to a set list of IP's where we assume the leaders are.
	/// If none of them respond, we will assume we are the first and assume the role of leader.
	/// </summary>
	/// <param name="assumeLeader">
	/// If set to true, will skip the connect phase and assume that this node is the leader.
	/// </param>
	void start(RequestHandler* requestHandler, 
		std::vector<std::pair<std::string, std::string>> ips, 
		bool assumeLeader = false);

	/// <summary>
	/// Returns true if this node is the leader in the network.
	/// </summary>
	virtual bool isLeader() { return leader; };

	/// <summary>
	/// Will pass the given request on to the leader of the network.
	/// </summary>
	/// <returns> The string that the leader gives back. </returns>
	virtual std::string passRequestToLeader(std::string requestType, std::string request);

	/// <summary>
	/// Will handle a connect request by a new node that wants to join the network.
	/// If this node is the leader, we will add it to our list of connections.
	/// If we are not the leader, we will return the ip and port of the leader of the network.
	/// </summary>
	/// <returns> 
	/// If we are the leader, we will return ok and the initial data.
	///	If we are not the leader, we will return the leader of the network.
	/// </returns>
	virtual std::string connectNewNode(boost::shared_ptr<TcpConnection> connection, std::string request);

	/// <summary>
	/// Reads the given file and gets the ips out of it.
	/// </summary>
	/// <returns>List of ips with port.</returns>
	virtual std::vector<std::pair<std::string, std::string>> getIps(std::string file = ".env");
private:
	/// <summary>
	/// Will try to set up a connection with the leader.
	/// If no connection can be astablished, we will assume the role of leader.
	/// </summary>
	/// <param name="ips"> List of node to try and connect with. </param>
	void connectToLeader(std::vector<std::pair<std::string, std::string>> ips);


	/// <summary>
	/// Listens for the heartbeat on the connection with the leader.
	/// The heartbeat will be passed on to the handleHeartbeat method once one is received.
	/// </summary>
	void listenForHeartbeat();

	/// <summary>
	/// Parses the data that has been passed on by the leader through the heartbeat.
	/// </summary>
	void handleHeartbeat(std::string heartbeat);

	/// <summary>
	/// Handles the initial data that is send back by the leader.
	/// </summary>
	/// <param name="initialData">The initial data. 
	/// Position 0 is OK, positions 1 and 2 are the ip and port of this node respectively.
	/// The rest are the other nodes in the network.</param>
	void handleInitialData(std::vector<std::string> initialData);

	/// <summary>
	/// Will try to connect to the given Ip and port.
	/// If we connect but we get a different ip and port back,
	/// then we will give those back through the ip and port variables.
	/// The response will hold the full response we got.
	/// </summary>
	void tryConnectingWithIp(std::string &ip, std::string &port, std::string &response);


	/// <summary>
	/// Sends a heartbeat to all nodes in the network every once in a while.
	/// </summary>
	void heartbeatSender();

	/// <summary>
	/// Gets the data that is going to be send in the heartbeat by the heartbeatSender method.
	/// </summary>
	std::string getHeartbeat();

	/// <summary>
	/// Listens for incomming data on the given connection. Any request received will be handled localy.
	/// The result of the request will be send back through the connection. This will be done as long
	/// as the connection stays open.
	/// </summary>
	void listenForRequests(boost::shared_ptr<TcpConnection> connection);

	/// <summary>
	/// Removes a connection from the list of nodes connected to the leader.
	/// This method will keep track that the connection has been dropped, so that it can be send in the heartbeat.
	/// </summary>
	/// <param name="i">The index of the node to be dropped.</param>
	void dropConnection(int i);


	/// <summary>
	/// Converts a connection to a string that can be used to connect to the other side of the connection.
	/// </summary>
	std::string connectionToString(boost::shared_ptr<TcpConnection> connection, std::string port);

	bool leader;
	bool stop = false;
	bool started = false;
	std::mutex mtx;

	// Non-leader variables.
	NetworkHandler* networkhandler;
	std::string leaderIp, leaderPort, myIp, myPort;
	std::vector<std::pair<std::string, std::string>> nonLeaderNodes;

	// Leader variables.
	std::vector<std::pair<boost::shared_ptr<TcpConnection>, std::string>>* others;
	RequestHandler* requestHandler;
	std::string nodeConnectionChange = "";
};

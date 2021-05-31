/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once

#include "RequestHandler.h"
#include "RAFTConsensus.h"

#include <string>
#include <vector>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <iostream>

#define PORT 8003
#define CONNECTION_TIMEOUT 10000000	// Timeout in microseconds.

using boost::asio::ip::tcp;

class TcpServer;

/// <summary>
/// Handles connections with database.
/// </summary>
class ConnectionHandler
{
public:
	~ConnectionHandler();
	/// <summary>
	/// Starts listening for requests. Takes in a pointer to the database handler.
	/// </summary>
	void startListen(DatabaseHandler* databaseHandler, DatabaseConnection* databaseConnection, RAFTConsensus* raft, int port = PORT, RequestHandler *handler = nullptr);

	RequestHandler* getRequestHandler() { return handler; };
private:
	RequestHandler* handler;
	TcpServer* server;
};

class TcpConnection
	: public boost::enable_shared_from_this<TcpConnection>
{
public:
	typedef boost::shared_ptr<TcpConnection> pointer;

	/// <summary>
	/// Creates the pointer to the tcp connection. Takes in the io context.
	/// </summary>
	static pointer create(boost::asio::io_context& ioContext);

	tcp::socket& socket()
	{
		return socket_;
	}


	/// <summary>
	/// Starts the handeling of a request. Takes in the request handler to call.
	/// </summary>
	void start(RequestHandler *handler, pointer thisPointer);

private:
	/// <summary>
	/// Loops until expected amount of data is received.
	/// </summary>
	void readExpectedData(int& size, std::vector<char>& data, std::string& totalData, boost::system::error_code& error);

	TcpConnection(boost::asio::io_context& ioContext)
		: socket_(ioContext)
	{
	}

	tcp::socket socket_;
	std::string message_;
};

class TcpServer
{
public:
	TcpServer(boost::asio::io_context& ioContext, DatabaseHandler* databaseHandler, DatabaseConnection* databaseConnection, RAFTConsensus* raft, RequestHandler* handler, int port);

	void stop();
private:

	/// <summary>
	/// Starts accepting incoming requests.
	/// </summary>
	void startAccept();

	/// <summary>
	/// Handles the accepting of an incoming request.
	/// Takes in the pointer to the tcp connection and a pointer to an error to write to.
	/// </summary>
	void handleAccept(TcpConnection::pointer newConnection, const boost::system::error_code& error);

	boost::asio::io_context& ioContext_;
	tcp::acceptor acceptor_;
	RequestHandler* handler;

	bool stopped = false;
};

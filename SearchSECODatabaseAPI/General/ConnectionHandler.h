/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include "RequestHandler.h"
#include "RAFTConsensus.h"

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

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
	/// <param name="databaseHandler"> Handles the connection with the database for the main keyspace. </param>
	/// <param name="databaseConnection"> Handles the connection with the database for the job keyspace. </param>
	/// <param name="raft"> The raft system for the job queue. </param>
	/// <param name="port"> The port on which we will listen. </param>
	/// <param name="handler">
	/// The request handler that will be used. A new request handler will be made if the given request handler 
	/// is a nullptr.
	/// </param>
	void startListen(DatabaseHandler* databaseHandler, 
		DatabaseConnection* databaseConnection, 
		RAFTConsensus* raft, 
		int port = PORT, 
		RequestHandler *handler = nullptr);

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
	/// Gets the ip of the other side of this connection.
	/// </summary>
	virtual std::string getIp();

	/// <summary>
	/// Sends the given data to the other side of the connection.
	/// </summary>
	virtual void sendData(const std::string &data, boost::system::error_code &error);

	/// <summary>
	/// Starts the handeling of a request. Takes in the request handler to call.
	/// </summary>
	virtual void start(RequestHandler *handler, pointer thisPointer);

protected:
	/// <summary>
	/// Constructor. Not public because you need to use the create method.
	/// Not private because we need this constructor for the mock.
	/// </summary>
	TcpConnection(boost::asio::io_context& ioContext)
		: socket_(ioContext)
	{
	}

private:
	/// <summary>
	/// Loops until expected amount of data is received.
	/// </summary>
	void readExpectedData(int &size, std::vector<char> &data, std::string &totalData, 
						  boost::system::error_code &error);


	tcp::socket socket_;
	std::string message_;
};

class TcpServer
{
public:
	TcpServer(boost::asio::io_context &ioContext, DatabaseHandler *databaseHandler,
			  DatabaseConnection *databaseConnection, RAFTConsensus *raft, RequestHandler *handler, int port);

	/// <summary>
	/// Stops the server.
	/// </summary>
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

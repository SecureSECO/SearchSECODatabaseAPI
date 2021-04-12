/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
� Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include "RequestHandler.h"
#include <string>
#include <vector>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <iostream>

using boost::asio::ip::tcp;

/// <summary>
/// Handles connections with database.
/// </summary>
class ConnectionHandler
{
public:
	/// <summary>
	/// Starts listening for requests. Takes in a pointer to the database handler.
	/// </summary>
	void startListen(DatabaseHandler* databaseHandler);
private:
	RequestHandler handler;
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
	void start(RequestHandler handler);

private:
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
	TcpServer(boost::asio::io_context& ioContext, DatabaseHandler* databaseHandler);

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
	RequestHandler handler;
};

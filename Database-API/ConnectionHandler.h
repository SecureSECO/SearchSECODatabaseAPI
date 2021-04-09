/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
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
	void StartListen(DatabaseHandler* databaseHandler);
private:
	RequestHandler handler;
};

class tcp_connection
	: public boost::enable_shared_from_this<tcp_connection>
{
public:
	typedef boost::shared_ptr<tcp_connection> pointer;

	static pointer create(boost::asio::io_context& io_context);

	tcp::socket& socket()
	{
		return socket_;
	}

	void start(RequestHandler handler);

private:
	tcp_connection(boost::asio::io_context& io_context)
		: socket_(io_context)
	{
	}

	void handle_read(const boost::system::error_code& /*error*/,
		size_t /*bytes_transferred*/)
	{
	}

	tcp::socket socket_;
	std::string message_;
};

class tcp_server
{
public:
	tcp_server(boost::asio::io_context& io_context, DatabaseHandler* databaseHandler);

private:
	void start_accept();

	void handle_accept(tcp_connection::pointer new_connection,
		const boost::system::error_code& error);

	boost::asio::io_context& io_context_;
	tcp::acceptor acceptor_;
	RequestHandler handler;
};

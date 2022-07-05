/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "ConnectionHandler.h"
#include "Utility.h"
#include "HTTPStatus.h"

#include <boost/array.hpp>
#include <boost/bind/bind.hpp>
#include <chrono>
#include <string>
#include <vector>
#include <iostream>

ConnectionHandler::~ConnectionHandler() 
{
	server->stop();
}

void ConnectionHandler::startListen(DatabaseHandler *databaseHandler, DatabaseConnection *databaseConnection,
									RAFTConsensus *raft, Statistics *stats, int port,
									RequestHandler *requestHandler)
{
	if (requestHandler == nullptr)
	{
		handler = new RequestHandler();
	}
	else
	{
		handler = requestHandler;
	}
	try
	{
		boost::asio::io_context ioContext;
		std::vector<std::pair<std::string, std::string>> ips = raft->getIps();
		TcpServer server(ioContext, databaseHandler, databaseConnection, raft, handler, port, stats);
		this->server = &server;
		raft->start(handler, ips);
		ioContext.run();
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
}

// TCP Connection Methods.
TcpConnection::pointer TcpConnection::create(boost::asio::io_context& ioContext)
{
	return pointer(new TcpConnection(ioContext));
}

void TcpConnection::sendData(const std::string &data, boost::system::error_code &error) 
{
	boost::asio::write(socket_, boost::asio::buffer(data), error);
}

void TcpConnection::start(RequestHandler *handler, pointer thisPointer, Statistics *stats)
{

	std::vector<char> request = std::vector<char>();
	boost::system::error_code error;
	size_t len;
	len = boost::asio::read_until(socket_, boost::asio::dynamic_buffer(request), '\n', error);
	if (error == boost::asio::error::eof)
	{
		// The socket was closed before receiving '\n'.
		return;
	}
	std::string r(request.begin(), request.begin() + len - 1);

	std::cout << r << std::endl;
	std::vector<std::string> header = Utility::splitStringOn(r, FIELD_DELIMITER_CHAR);
	if (header.size() < 3)
	{
		boost::asio::write(socket_, boost::asio::buffer(HTTPStatusCodes::clientError("Header too short.")), error);
		return;
	}

	std::string length = header[2];
	std::string totalData(request.begin() + len, request.end());

	int size = Utility::safeStoi(length) - (request.size() - len);
	if (errno != 0)
	{
		boost::asio::write(socket_, boost::asio::buffer(HTTPStatusCodes::clientError("Error parsing command.")), error);
		return;
	}
	if (size < 0)
	{
		boost::asio::write(
			socket_, boost::asio::buffer(HTTPStatusCodes::clientError("Request body larger than expected.")), error);
		return;
	}

	stats->requestCounter->Add({{"Node", stats->myIP}, {"Client", header[1]}, {"Request", header[0]}}).Increment();
	stats->latestRequest->Add({{"Node", stats->myIP}, {"Client", header[1]}, {"Request", header[0]}}).SetToCurrentTime();
	std::vector<char> data(size);
	readExpectedData(size, data, totalData, error);
	std::string result = handler->handleRequest(header[0], header[1], totalData, thisPointer);
	stats->newRequest = true;
	boost::asio::write(socket_, boost::asio::buffer(result), error);
}

void TcpConnection::readExpectedData(int &size, std::vector<char> &data, std::string &totalData,
									 boost::system::error_code &error)
{
	while (size > 0)
	{
		int prevSize = size;
		size -= socket_.read_some(boost::asio::buffer(data), error);
		if (error == boost::asio::error::eof)
		{
			// The socket was closed before receiving all data.
			return;
		}
		if (size < 0)
		{
			boost::asio::write(socket_, boost::asio::buffer(std::string("Request body larger than expected.")), error);
			return;
		}
		totalData.append(std::string(data.begin(), data.begin() + prevSize - size));
	}
}

// TCP Server Methods.
TcpServer::TcpServer(boost::asio::io_context& ioContext, 
	DatabaseHandler* databaseHandler, 
	DatabaseConnection* databaseConnection, 
	RAFTConsensus* raft, RequestHandler* handler, 
	int port, Statistics *stats)
	: ioContext_(ioContext),
	acceptor_(ioContext, tcp::endpoint(tcp::v4(), port)), stats(stats)
{
	this->handler = handler;
	stats->myIP = raft->getMyIP();
	handler->initialize(databaseHandler, databaseConnection, raft, stats);
	startAccept();
}

void TcpServer::startAccept()
{
	TcpConnection::pointer newConnection = TcpConnection::create(ioContext_);

	acceptor_.async_accept(newConnection->socket(),
		boost::bind(&TcpServer::handleAccept, this, newConnection,
			boost::asio::placeholders::error));
}

void TcpServer::handleAccept(TcpConnection::pointer newConnection, const boost::system::error_code &error)
{
	if (stopped) 
	{
		return;
	}
	startAccept();
	if (!error)
	{
		new std::thread(&TcpConnection::start, newConnection, handler, newConnection, stats);
	}

}

void TcpServer::stop() 
{
	stopped = true;
	acceptor_.cancel();
}
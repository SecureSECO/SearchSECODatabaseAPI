/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "ConnectionHandler.h"
#include "Utility.h"
#include <chrono>

ConnectionHandler::~ConnectionHandler() 
{
	server->stop();
}

// Connection Handler Methods.
void ConnectionHandler::startListen(DatabaseHandler* databaseHandler, 
	DatabaseConnection* databaseConnection, 
	RAFTConsensus* raft, 
	int port, 
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
		TcpServer server(ioContext, databaseHandler, databaseConnection, raft, handler, port);
		this->server = &server;
		ioContext.run();
		
	}
	catch (std::exception& e)
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

void TcpConnection::start(RequestHandler* handler, pointer thisPointer)
{
	
	std::vector<char> request = std::vector<char>();
	boost::system::error_code error;
	size_t len;
	len = boost::asio::read_until(socket_, boost::asio::dynamic_buffer(request), '\n', error);
	if (error == boost::asio::error::eof)
	{
		// Socket was closed before receiving \n.
		return;
	}
	std::string r(request.begin(), request.begin() + len - 1);

	std::cout << r << std::endl;
	std::string length = r.substr(4);
	std::string totalData(request.begin() + len, request.end());

	int size = Utility::safeStoi(length) - (request.size() - len);
	if (errno != 0)
	{
		boost::asio::write(socket_, boost::asio::buffer(std::string("Error parsing command.")), error);
		return;
	}
	if (size < 0)
	{
		boost::asio::write(socket_, boost::asio::buffer(std::string("Request body larger than expected.")), error);
		return;
	}
	std::vector<char> data(size);
	readExpectedData(size, data, totalData, error);
	std::string result = handler->handleRequest(r.substr(0, 4), totalData, thisPointer);
	boost::asio::write(socket_, boost::asio::buffer(result), error);
}

void TcpConnection::readExpectedData(int& size, std::vector<char>& data, std::string& totalData, boost::system::error_code& error)
{
	while (size > 0)
	{
		int prevSize = size;
		size -= socket_.read_some(boost::asio::buffer(data), error);
		if (error == boost::asio::error::eof)
		{
			// Socket was closed before receiving all data..
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

// TCP server Methods.
TcpServer::TcpServer(boost::asio::io_context& ioContext, 
	DatabaseHandler* databaseHandler, 
	DatabaseConnection* databaseConnection, 
	RAFTConsensus* raft, RequestHandler* handler, 
	int port)
	: ioContext_(ioContext),
	acceptor_(ioContext, tcp::endpoint(tcp::v4(), port))
{
	this->handler = handler;
	handler->initialize(databaseHandler, databaseConnection, raft);
	startAccept();
}

void TcpServer::startAccept()
{
	TcpConnection::pointer newConnection = TcpConnection::create(ioContext_);

	acceptor_.async_accept(newConnection->socket(),
		boost::bind(&TcpServer::handleAccept, this, newConnection,
			boost::asio::placeholders::error));
}

void TcpServer::handleAccept(TcpConnection::pointer newConnection,
	const boost::system::error_code& error)
{
	if (stopped) 
	{
		return;
	}
	startAccept();
	if (!error)
	{
		newConnection->start(handler, newConnection);
	}

}

void TcpServer::stop() 
{
	stopped = true;
	acceptor_.cancel();
}


std::string TcpConnection::getIp() 
{
	return socket_.remote_endpoint().address().to_string();
}
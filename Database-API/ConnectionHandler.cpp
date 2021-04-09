
#include "ConnectionHandler.h"



// Connection Handler Methods
void ConnectionHandler::startListen(DatabaseHandler* databaseHandler)
{

	try
	{
		boost::asio::io_context ioContext;
		tcp_server server(ioContext, databaseHandler);
		ioContext.run();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}


// TCP Connection Methods
tcp_connection::pointer tcp_connection::create(boost::asio::io_context& ioContext)
{
	return pointer(new tcp_connection(ioContext));
}

void tcp_connection::start(RequestHandler handler)
{

	/*boost::asio::async_read(socket_, boost::asio::buffer(request),
		boost::bind(&tcp_connection::handle_read, shared_from_this(),
		  boost::asio::placeholders::error,
		  boost::asio::placeholders::bytes_transferred));*/
	std::vector<char> request = std::vector<char>();
	boost::system::error_code error;

	size_t len = boost::asio::read_until(socket_, boost::asio::dynamic_buffer(request), '\n');

	std::string r(request.begin(), request.begin() + len - 1);

	std::cout << r << std::endl;

	std::string length = r.substr(4);

	std::string totalData(request.begin() + len, request.end());

	int size = stoi(length) - (request.size() - len);
	std::vector<char> data(size);
	while (size > 0)
	{
		int prevSize = size;

		size -= socket_.read_some(boost::asio::buffer(data), error);
		totalData.append(std::string(data.begin(), data.begin() + prevSize - size));
	}

	std::string result = handler.handleRequest(r.substr(0, 4), totalData);

	boost::asio::write(socket_, boost::asio::buffer(result), error);

	/*boost::asio::async_write(socket_, boost::asio::buffer(result),
		boost::bind(&tcp_connection::handle_read, shared_from_this(),
		  boost::asio::placeholders::error,
		  boost::asio::placeholders::bytes_transferred));*/
}


// TCP server Methods

tcp_server::tcp_server(boost::asio::io_context& ioContext, DatabaseHandler* databaseHandler)
	: io_context_(ioContext),
	acceptor_(ioContext, tcp::endpoint(tcp::v4(), 8003))
{
	handler.initialize(databaseHandler);
	startAccept();
}

void tcp_server::startAccept()
{
	tcp_connection::pointer newConnection =
		tcp_connection::create(io_context_);

	acceptor_.async_accept(newConnection->socket(),
		boost::bind(&tcp_server::handleAccept, this, newConnection,
			boost::asio::placeholders::error));
}

void tcp_server::handleAccept(tcp_connection::pointer newConnection,
	const boost::system::error_code& error)
{
	startAccept();
	if (!error)
	{
		newConnection->start(handler);
	}

}

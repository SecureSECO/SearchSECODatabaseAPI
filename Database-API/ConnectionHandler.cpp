
#include "ConnectionHandler.h"



// Connection Handler Methods
void ConnectionHandler::StartListen()
{

	try
	{
		boost::asio::io_context io_context;
		tcp_server server(io_context);
		io_context.run();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}


// TCP Connection Methods 
tcp_connection::pointer tcp_connection::create(boost::asio::io_context& io_context)
{
	return pointer(new tcp_connection(io_context));
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

	//size_t* length;
	//sscanf(r.c_str(), "%zu", length);

	//std::vector<int>::size_type size = length;

	std::string length = r.substr(4);

	int size = stoi(length) - (request.size() - len);
	std::vector<char> data(size);
	if (size > 0)
	{
		socket_.read_some(boost::asio::buffer(data), error);
	}

	std::string d(data.begin(), data.end());

	std::string result = handler.HandleRequest(r.substr(0, 4), std::string(request.begin() + len, request.end()) + d);

	boost::asio::write(socket_, boost::asio::buffer(result), error);

	/*boost::asio::async_write(socket_, boost::asio::buffer(result),
		boost::bind(&tcp_connection::handle_read, shared_from_this(),
		  boost::asio::placeholders::error,
		  boost::asio::placeholders::bytes_transferred));*/
}


// TCP server Methods

tcp_server::tcp_server(boost::asio::io_context& io_context)
	: io_context_(io_context),
	acceptor_(io_context, tcp::endpoint(tcp::v4(), 13))
{
	handler.Initialize();
	start_accept();
}

void tcp_server::start_accept()
{
	tcp_connection::pointer new_connection =
		tcp_connection::create(io_context_);

	acceptor_.async_accept(new_connection->socket(),
		boost::bind(&tcp_server::handle_accept, this, new_connection,
			boost::asio::placeholders::error));
}

void tcp_server::handle_accept(tcp_connection::pointer new_connection,
	const boost::system::error_code& error)
{
	start_accept();
	if (!error)
	{
		new_connection->start(handler);
	}

}
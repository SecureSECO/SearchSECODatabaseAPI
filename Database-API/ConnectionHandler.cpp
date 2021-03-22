#include <iostream>
#include <string>
#include <vector>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include "RequestHandler.h"
#include "ConnectionHandler.h"

using boost::asio::ip::tcp;

class tcp_connection
  : public boost::enable_shared_from_this<tcp_connection>
{
public:
  typedef boost::shared_ptr<tcp_connection> pointer;

  static pointer create(boost::asio::io_context& io_context)
  {
    return pointer(new tcp_connection(io_context));
  }

  tcp::socket& socket()
  {
    return socket_;
  }

  void start(RequestHandler handler)
  {

	/*boost::asio::async_read(socket_, boost::asio::buffer(request),
		boost::bind(&tcp_connection::handle_read, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));*/
	std::vector<char> request(128);
	boost::system::error_code error;

	size_t len = socket_.read_some(boost::asio::buffer(request), error);

	std::string r(request.begin(), request.end());

	//size_t* length;
	//sscanf(r.c_str(), "%zu", length);

	//std::vector<int>::size_type size = length;

	std::string length = r.substr(4);

	int size = stoi(length);

	std::vector<char> data(size);

	socket_.read_some(boost::asio::buffer(data), error);

	std::string d(data.begin(), data.end());

	std::string result = handler.HandleRequest(r.substr(0, 3), d);

	boost::asio::write(socket_, boost::asio::buffer(result), error);

    /*boost::asio::async_write(socket_, boost::asio::buffer(result),
        boost::bind(&tcp_connection::handle_read, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));*/
  }

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
  tcp_server(boost::asio::io_context& io_context)
    : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), 13))
  {
	handler.Initialize();
    start_accept();
  }

private:
  void start_accept()
  {
    tcp_connection::pointer new_connection =
      tcp_connection::create(io_context_);

    acceptor_.async_accept(new_connection->socket(),
        boost::bind(&tcp_server::handle_accept, this, new_connection,
          boost::asio::placeholders::error));
  }

  void handle_accept(tcp_connection::pointer new_connection,
      const boost::system::error_code& error)
  {
    if (!error)
    {
      new_connection->start(handler);
    }

    start_accept();
  }

  boost::asio::io_context& io_context_;
  tcp::acceptor acceptor_;
	RequestHandler handler;
};

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

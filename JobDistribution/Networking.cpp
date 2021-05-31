/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

// Controller includes
#include "Networking.h"

// External includes
#include <boost/array.hpp>
#include <iostream>


// https://www.boost.org/doc/libs/1_75_0/doc/html/boost_asio/tutorial.html was used as a base.

void NetworkHandler::openConnection(std::string server, std::string port)
{
	std::string serverApi = server;

	tcp::resolver resolver(ioContext);
	tcp::resolver::results_type endpoints = resolver.resolve(serverApi, port);

	boost::asio::connect(socket, endpoints);
}

NetworkHandler* NetworkHandler::createHandler()
{
	return new NetworkHandler();
}

void NetworkHandler::sendData(const char* data, int dataLength)
{
	boost::asio::write(socket, boost::asio::buffer(data, dataLength));
}

std::string NetworkHandler::receiveData(bool stopOnNewLine)
{
	// The buffer we are going to return as a string.
	std::vector<char> ret = std::vector<char>();
	for (;;)
	{

		boost::array<char, 128> buf;
		boost::system::error_code error;

		// Read incomming data.
		size_t len = socket.read_some(boost::asio::buffer(buf), error);
		if (len == 0 && stopOnNewLine) 
		{
			throw std::runtime_error("No data received, meaning the other side dropped out.");
		}

		// Add it to out buffer.
		for (int i = 0; i < len; i++)
		{
			ret.push_back(buf[i]);
		}

		if (ret[ret.size()-1] == '\n' && stopOnNewLine || error == boost::asio::error::eof && !stopOnNewLine)
		{
			break; // Connection closed cleanly by peer.
		}
		else if (error)
		{
			// If any error occures, we just throw.
			std::cout << "Networking error: " << error.message();
			throw boost::system::system_error(error);
		}

	}
	if (stopOnNewLine) 
	{
		return std::string(ret.begin(), ret.end() - 1);
	}
	return std::string(ret.begin(), ret.end());
}

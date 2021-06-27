/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "RequestHandler.h"
#include "ConnectionHandler.h"

#include <string>
#include <gmock/gmock.h>

/// <summary>
/// Handles requests.
/// </summary>
class TcpConnectionMock : public TcpConnection
{
public:

	MOCK_METHOD(void, sendData, (const std::string &data, boost::system::error_code &error));
	MOCK_METHOD(void, start, (RequestHandler *handler, pointer thisPointer), ());
	MOCK_METHOD(std::string, getIp, (), ());
	
	TcpConnectionMock(boost::asio::io_context &iocon) : TcpConnection(iocon)
	{
		ON_CALL(*this, start).WillByDefault([this](RequestHandler *handler, pointer thisPointer) 
		{
			usleep(500000);
		});	
		ON_CALL(*this, getIp).WillByDefault([this]() {return "127.0.0.1";});
	};
};


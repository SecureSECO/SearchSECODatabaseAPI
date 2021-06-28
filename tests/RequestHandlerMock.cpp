/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "RequestHandler.h"
#include "ConnectionHandler.h"

#include <string>
#include <boost/shared_ptr.hpp>
#include <gmock/gmock.h>

/// <summary>
/// Handles requests.
/// </summary>
class RequestHandlerMock : public RequestHandler
{
public:

	MOCK_METHOD(void, initialize, 
		(DatabaseHandler *databaseHandler, 
		DatabaseConnection *databaseConnection, 
		RAFTConsensus* raft, 
		std::string ip, 
		int port), 
		());
	MOCK_METHOD(std::string, handleRequest, 
		(std::string requestType, 
		std::string request, 
		boost::shared_ptr<TcpConnection> connection), 
		());

};

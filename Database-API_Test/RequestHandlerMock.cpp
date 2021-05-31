/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "gmock/gmock.h"
#include "RequestHandler.h"
#include <string>
#include <boost/shared_ptr.hpp>

/// <summary>
/// Handles requests.
/// </summary>
class RequestHandlerMock : public RequestHandler
{
public:
	MOCK_METHOD(void, initialize, (DatabaseHandler *databaseHandler, DatabaseConnection *databaseConnection, RAFTConsensus* raft, std::string ip, int port), ());
	MOCK_METHOD(std::string, handleRequest, (std::string requestType, std::string request, boost::shared_ptr<TcpConnection> connection), ());
};

class RequestHandlerMock2 : public RequestHandler 
{
public:
	void initialize(DatabaseHandler *databaseHandler, DatabaseConnection *databaseConnection, RAFTConsensus* raft, std::string ip = IP, int port = DBPORT) override {};
	std::string handleRequest(std::string requestType, std::string request, boost::shared_ptr<TcpConnection> connection) override {return "ok?127.0.0.1?8003\n";};
	//std::string test() override {return "ok?127.0.0.1?8003\n";};	
};
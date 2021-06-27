/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "Types.h"

#include <string>
#include <boost/shared_ptr.hpp>
#include <gmock/gmock.h>
using namespace types;

/// <summary>
/// Handles interaction with database.
/// </summary>
class MockRaftConsensus : public RAFTConsensus
{
public:
	MOCK_METHOD(bool, isLeader, (), ());
	MOCK_METHOD(std::string, passRequestToLeader, (std::string requestType, std::string request), ());
	MOCK_METHOD(std::string, connectNewNode, (boost::shared_ptr<TcpConnection> connection, std::string request), ());
};


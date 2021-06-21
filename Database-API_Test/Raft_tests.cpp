/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "RAFTConsensus.h"
#include "Networking.h"
#include "RequestHandler.h"
#include "RequestHandlerMock.cpp"
#include "ConnectionHandler.h"
#include "ConnectionMock.cpp"
#include "JDDatabaseMock.cpp"
#include "DatabaseMock.cpp"

#include <gtest/gtest.h>

#define TESTLISTENPORT 9043
#define TESTIP "127.0.0.1"

std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);
std::string entryDelimiter(1, ENTRY_DELIMITER_CHAR);

MATCHER_P(matcherNotNull, request, "")
{
	return arg != nullptr;
}

TEST(RaftTests, BecomeLeader)
{
	RequestHandler raftHandler; 
	MockDatabase database;
	MockJDDatabase jddatabase;
	raftHandler.initialize(&database, &jddatabase, nullptr);
	{
		RAFTConsensus raft;
		raft.start(&raftHandler, {{TESTIP, "-1"}}, false);

		ASSERT_TRUE(raft.isLeader());
	}
}

TEST(RaftTests, AcceptConnection)
{
	boost::asio::io_context ioCon;
	RequestHandler handler; 
	MockDatabase database;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, nullptr);
	TcpConnectionMock* connMock = new TcpConnectionMock(ioCon);
	{
		RAFTConsensus raft;
		raft.start(&handler, {}, true);

		ASSERT_TRUE(raft.isLeader());

		TcpConnection::pointer connection = TcpConnection::pointer(connMock);
		boost::system::error_code err;

		std::string resp = raft.connectNewNode(connection, "-1" + entryDelimiter);

		EXPECT_EQ(resp, RESPONSE_OK + fieldDelimiter + TESTIP + fieldDelimiter + "-1" + entryDelimiter);
	}
	delete connMock;
}

TEST(RaftTests, ReadIpsFromFile) 
{
	RAFTConsensus raft;
	auto ips = raft.getIps("dotenvTestfile.txt");
	std::string port = std::to_string(PORT);
	std::vector<std::pair<std::string, std::string>> expectedOutput = {{"127.0.0.1", port}, {"127.0.0.2", port}};
	ASSERT_EQ(ips, expectedOutput);
}

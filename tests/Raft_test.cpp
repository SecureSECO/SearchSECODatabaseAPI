/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "TestDefinitions.h"
#include "Definitions.h"
#include "RAFTConsensus.h"
#include "Networking.h"
#include "RequestHandler.h"
#include "RequestHandlerMock.cpp"
#include "ConnectionHandler.h"
#include "ConnectionMock.cpp"
#include "JDDatabaseMock.cpp"
#include "DatabaseMock.cpp"

#include <gtest/gtest.h>

std::string fieldDelimiter(1, FIELD_DELIMITER_CHAR);
std::string entryDelimiter(1, ENTRY_DELIMITER_CHAR);

MATCHER_P(matcherNotNull, request, "")
{
	return arg != nullptr;
}

// Check if we can become a leader.
TEST(RaftTests, BecomeLeader)
{
	// Set up the test.
	errno = 0;

	MockDatabase database;
	MockJDDatabase jddatabase;
	RequestHandler raftHandler;
	raftHandler.initialize(&database, &jddatabase, nullptr, nullptr);
	{
		RAFTConsensus raft(nullptr);
		raft.start(&raftHandler, {{TEST_IP, "-1"}}, false);

		ASSERT_TRUE(raft.isLeader());
	}
}

// Check if a connection with a new node be accepted.
TEST(RaftTests, AcceptConnection)
{
	// Set up the test.
	errno = 0;

	boost::asio::io_context ioCon;
	MockDatabase database;
	MockJDDatabase jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr, nullptr);

	TcpConnectionMock* connMock = new TcpConnectionMock(ioCon);
	{
		RAFTConsensus raft(nullptr);
		raft.start(&handler, {}, true);

		ASSERT_TRUE(raft.isLeader());

		TcpConnection::pointer connection = TcpConnection::pointer(connMock);
		boost::system::error_code err;

		std::string resp = raft.connectNewNode(connection, "127.0.0.1" + fieldDelimiter + "-1" + entryDelimiter);

		EXPECT_EQ(resp, RESPONSE_OK + fieldDelimiter + TEST_IP + fieldDelimiter + "-1" + entryDelimiter);
	}
	delete connMock;
}

// Test whether the current ips are returned correctly.
TEST(RaftTests, GetCurrentIPs)
{
	// Set up the test.
	errno = 0;

	boost::asio::io_context ioCon;
	MockDatabase database;
	MockJDDatabase jddatabase;
	RequestHandler handler;
	handler.initialize(&database, &jddatabase, nullptr, nullptr);

	TcpConnectionMock *connMock = new TcpConnectionMock(ioCon);
	{
		RAFTConsensus raft(nullptr);
		raft.start(&handler, {}, true);

		ASSERT_TRUE(raft.isLeader());

		TcpConnection::pointer connection = TcpConnection::pointer(connMock);
		boost::system::error_code err;

		std::string resp = raft.connectNewNode(connection, "127.0.0.2" + fieldDelimiter + "-1" + entryDelimiter);

		std::vector<std::string> ips = raft.getCurrentIPs();

		std::vector<std::string> expectedOutput = {"127.0.0.2" + fieldDelimiter + "-1", "?"};

		ASSERT_EQ(ips, expectedOutput);
	}
	delete connMock;
}

// Test if we can read the ips from a file.
TEST(RaftTests, ReadIpsFromFile) 
{
	// Set up the test.
	errno = 0;

	RAFTConsensus raft(nullptr);
	
	auto ips = raft.getIps("dotenvTestfile.txt");
	std::string port = std::to_string(PORT);
	std::vector<std::pair<std::string, std::string>> expectedOutput = {{"127.0.0.1", port}, {"127.0.0.2", port}};

	// Check if the output is as expected.
	ASSERT_EQ(ips, expectedOutput);
}

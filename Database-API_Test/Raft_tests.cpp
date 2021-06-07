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

TEST(RaftTests, AssumeLeaderTest) 
{
	RequestHandlerMock handler; 
	ConnectionHandler listen;
	std::thread* tread = new std::thread(&ConnectionHandler::startListen, &listen, nullptr, nullptr, nullptr, TESTLISTENPORT, &handler);
	usleep(500000); // Just to make sure the listner has started.

	EXPECT_CALL(handler, handleRequest("conn", "8003" + entryDelimiter, matcherNotNull(nullptr))).Times(0);

	RAFTConsensus raft;
	raft.start(nullptr, true, {{TESTIP, std::to_string(TESTLISTENPORT)}});

	ASSERT_TRUE(raft.isLeader());
}

TEST(RaftTests, ConnectToLeader) 
{
	RequestHandlerMock handler; 
	{
		ConnectionHandler listen;
		EXPECT_CALL(handler, handleRequest("conn", std::to_string(PORT) + entryDelimiter, matcherNotNull(nullptr))).Times(1)
			.WillOnce(testing::Return(RESPONSE_OK + (fieldDelimiter + TESTIP + fieldDelimiter + std::to_string(PORT) + entryDelimiter)));
		std::thread* tread = new std::thread(&ConnectionHandler::startListen, &listen, nullptr, nullptr, nullptr, TESTLISTENPORT, &handler);
		usleep(500000); // Just to make sure the listner has started.


		RAFTConsensus raft;
		raft.start(nullptr, false, {{TESTIP, std::to_string(TESTLISTENPORT)}});

		ASSERT_TRUE(!raft.isLeader());	
	}
	usleep(500000); // Extra wait to make sure the heartbeat is going to be send.

}

TEST(RaftTests, BecomeLeader) 
{
	RAFTConsensus raft;
	raft.start(nullptr, false, {{TESTIP, "-1"}});

	ASSERT_TRUE(raft.isLeader());
}

TEST(RaftTests, AcceptConnection) 
{
	boost::asio::io_context ioCon;
	TcpConnectionMock* connMock = new TcpConnectionMock(ioCon);
	{
		RAFTConsensus raft;
		raft.start(nullptr, true, {});

		ASSERT_TRUE(raft.isLeader());

		TcpConnection::pointer connection = TcpConnection::pointer(connMock);
		boost::system::error_code err;
		EXPECT_CALL((*connMock), sendData("A" + fieldDelimiter + (TESTIP "?-1") + entryDelimiter, err)).Times(1);

		std::string resp = raft.connectNewNode(connection, "-1" + entryDelimiter);
		
		EXPECT_EQ(resp, RESPONSE_OK + fieldDelimiter + TESTIP + fieldDelimiter + "-1" + entryDelimiter);
	}
	delete connMock;
}

TEST(RaftTests, PassRequestToLeader) 
{
	RequestHandler handler; 
	MockDatabase database;
	MockJDDatabase jddatabase;
	RAFTConsensus raftLeader;
	RAFTConsensus raftNonLeader;
	ConnectionHandler listen;
	
	EXPECT_CALL(jddatabase, getNumberOfJobs()).Times(1).WillOnce(testing::Return(1000));

	const std::string request = "gtjb";
	const std::string requestData = "data" + entryDelimiter;
	const std::string response = "response" + entryDelimiter;


	std::thread* tread = new std::thread(&ConnectionHandler::startListen, &listen, &database, &jddatabase, &raftLeader, TESTLISTENPORT, &handler);

	usleep(500000);
	raftLeader.start(&handler, false, {{TESTIP, "-1"}});
	raftNonLeader.start(nullptr, false, {{TESTIP, std::to_string(TESTLISTENPORT)}});

	ASSERT_TRUE(raftLeader.isLeader());
	ASSERT_TRUE(!raftNonLeader.isLeader());

	EXPECT_CALL(jddatabase, getTopJob()).Times(1).WillOnce(testing::Return(response));

	ASSERT_EQ(raftNonLeader.passRequestToLeader(request, requestData), HTTPStatusCodes::success("Spider" + fieldDelimiter + response));
	
}

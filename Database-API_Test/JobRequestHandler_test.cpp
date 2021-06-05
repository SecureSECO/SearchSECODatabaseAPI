/*This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)*/

#include "RequestHandler.h"
#include "DatabaseMock.cpp"
#include "JDDatabaseMock.cpp"
#include "RAFTConsensus.h"
#include "DatabaseConnection.h"
#include <gtest/gtest.h>

// Tests if the JobRequestHandler requests to connect to the database when initialized.
TEST(GeneralTest, InitializeJobTest)
{
	RequestHandler handler;
	MockDatabase database;
	MockJDDatabase jddatabase;
	EXPECT_CALL(jddatabase, connect(IP, DBPORT)).Times(1);
	handler.initialize(&database, &jddatabase, nullptr);
}


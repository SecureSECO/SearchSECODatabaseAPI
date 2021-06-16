﻿/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include <iostream>
#include <unistd.h>
#include "Types.h"
#include "Database-API.h"
#include "DatabaseHandler.h"
#include "ConnectionHandler.h"
#include "RAFTConsensus.h"

int main()
{
	RAFTConsensus raft;
	usleep(45000000);
	std::cout << "Connecting now" << std::endl;

	ConnectionHandler listen;
	DatabaseHandler databaseHandler;
	DatabaseConnection databaseConnection;

	listen.startListen(&databaseHandler, &databaseConnection, &raft);

	return 0;
}

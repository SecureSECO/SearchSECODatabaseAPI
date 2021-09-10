/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "Database-API.h"
#include "DatabaseConnection.h"
#include "DatabaseHandler.h"
#include "ConnectionHandler.h"
#include "RAFTConsensus.h"
#include "Statistics.h"

#include <iostream>
#include <unistd.h>

int main()
{
	std::cout << "Starting the API." << std::endl;

	Statistics stats;
	stats.Initialize();

	stats.readFromFile("stats/stats");

	new std::thread(&Statistics::synchronize, std::ref(stats), "stats/stats");

	RAFTConsensus raft(&stats);
	ConnectionHandler listen;
	DatabaseHandler databaseHandler;
	DatabaseConnection databaseConnection;

	listen.startListen(&databaseHandler, &databaseConnection, &raft, &stats);

	return 0;
}

/*
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

using namespace std;

int main()
{
	//usleep(45000000);
	cout << "Connecting now" << endl;

	RAFTConsensus raft;
	ConnectionHandler listen;
	DatabaseHandler databaseHandler;

	raft.start(listen.getRequestHandler());
	listen.startListen(&databaseHandler, &raft);

	return 0;
}

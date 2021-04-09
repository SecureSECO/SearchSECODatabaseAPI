/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include <iostream>
#include "Types.h"
#include "Database-API.h"
#include "DatabaseHandler.h"
#include "ConnectionHandler.h"

using namespace std;

int main()
{
	ConnectionHandler listen;
	DatabaseHandler databaseHandler;
	listen.startListen(&databaseHandler);

	return 0;
}

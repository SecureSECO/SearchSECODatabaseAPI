/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "DatabaseRequestHandler.h"
#include "Definitions.h"
#include "HTTPStatus.h"
#include "Utility.h"

#include <iostream>
#include <algorithm>
#include <sstream>
#include <ctime>
#include <regex>
#include <thread>
#include <future>
#include <utility>

DatabaseRequestHandler::DatabaseRequestHandler(DatabaseHandler *database, Statistics *stats, std::string ip, int port)
{
	this->database = database;
	this->stats = stats;
	connectWithRetry(ip, port);
	try
	{
		if (errno != 0)
		{	
			std::cout<<"Still in database connection."<<std::endl;
			std::cout<<"Error number: "<<errno<<std::endl;
			throw "Unable to connect to the database.";
		}
	}
	catch (const char *e)
	{
		std::cerr << e << std::endl;
	}
}

std::tuple<> DatabaseRequestHandler::connectWithRetry(std::string ip, int port)
{
	std::function<std::tuple<>()> function = [ip, port, this]()
	{
		this->database->connect(ip, port);
		return std::make_tuple();
	};
	return Utility::queryWithRetry<std::tuple<>>(function);
}
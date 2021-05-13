/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "JobRequestHandler.h"

JobRequestHandler::JobRequestHandler(DatabaseConnection *database, std::string ip, int port)
{
	this->database = database;
	database->connect(ip, port);
}

std::string JobRequestHandler::handleGetJobRequest()
{
	return database->getJob();
}

std::string JobRequestHandler::handleUploadJobRequest(std::string url)
{
	database->uploadJob(url);
	if(errno == 0)
	{
		return "Your job has been succesfully added to the queue.";
	}
	else
	{
		return "An error has occurred while adding your job to the queue.";
	}
}

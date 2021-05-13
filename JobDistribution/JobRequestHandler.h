/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include "DatabaseConnection.h"

class JobRequestHandler
{
public:
	JobRequestHandler(DatabaseConnection *database, std::string ip, int port);

	std::string handleUploadJobRequest(std::string);

	std::string handleGetJobRequest();

	DatabaseConnection *database;
};

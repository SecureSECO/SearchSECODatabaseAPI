/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include "RequestHandler.h"

/// <summary>
/// Handles connections with database.
/// </summary>
class ConnectionHandler
{
public:
	void StartListen();
private:
	RequestHandler handler;
};

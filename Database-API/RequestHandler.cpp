/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

#include "RequestHandler.h"

using namespace std;

/// <summary>
/// Handles requests towards database.
/// </summary>
class RequestHandler
{
	// Initialise the RequestHandler
	void Initialise()
	{
		// Set up a connection with the database using the ConnectionHandler.
		DatabaseHandler database
	    database.Connect();

		// Handle interaction with user using the DatabaseHandler.
		HandleRequests(database);
	}

	void HandleRequests(DatabaseHandler db)
	{
		// We listen for a request from the user.
		string request;
		getline(cin, request);

		// We handle the request based on its type.
		eRequestType eRequestType = requestToRequestType(request);
		request = request.substr(request.find(" ") + 1); // The type of the request is now removed from the string.
		switch (eRequestType)
		{
			case eAddProject:
				HandleAddProjectRequest(db, request);
			case eAddMethod:
				HandleAddMethodRequest(db, request);
			case eQuery:
				HandleQueryRequest(db, request);
			case eUnknown:
				HandleUnknownRequest();
		}

		// We recursively call the function for future requests.
		HandleRequests();
	}

	void HandleAddProjectRequest(DatabaseHandler db, string request)
	{
		Project project = JsonToProject(request);
		db.AddProject(project);
		return;
	}

	void HandleAddMethodRequest(DatabaseHandler db, string request)
	{
		Method method = JsonToMethod(request);
		db.AddMethod(method);
		return;
	}

	void HandleQueryRequest(DatabaseHandler db, string request)
	{
		return;
	}

	void HandleUnknownRequest()
	{
		cout << "Your request is not recognised.";
		return;
	}

	Project JsonToProject(string request)
	{
		// Todo: convert the project which is given in json format in the request to an actual project.
		Project project;
		return project;
	}

	Method JsonToMethod(string request)
	{
		// Todo: convert the method which is given in json format in the request to an actual method.
		Method method;
		return method;
	}

	// Determines the type of the request.
	eRequestType RequestToRequestType(string request)
	{
		string requestType = request.substr(0, request.find(" ")); // Gets first word of request, which determines its type.
		if (requestType == "addp")
			return eAddProject;
		else if (requestType == "addm")
			return eAddMethod;
		else if (requestType == "query")
			return eQuery;
		else return eUnknown;
	}
};

enum eRequestType
{
	eAddProject,
	eAddMethod,
	eQuery,
	eUnknown
};
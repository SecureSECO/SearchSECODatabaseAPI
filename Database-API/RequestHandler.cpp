/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include <iostream>
#include <string>
#include <json.hpp>

#include "RequestHandler.h"

using namespace std;

/// <summary>
/// Handles requests towards database.
/// </summary>
	// Initialise the RequestHandler
	void RequestHandler::Initialize()
	{
		// Set up a connection with the database using the ConnectionHandler.
		//DatabaseHandler database;
	    database.Connect();

		// Handle interaction with user using the DatabaseHandler.
		//HandleRequests(database);
	}

	string RequestHandler::HandleRequest(string request)
	{
		// We listen for a request from the user.
		//string request;
		//getline(cin, request);

		// We handle the request based on its type.
		eRequestType eRequestType = RequestToRequestType(request);
		request = request.substr(request.find(" ") + 1); // The type of the request is now removed from the string.
		string result;
		switch (eRequestType)
		{
			case eAddProject:
				HandleAddProjectRequest(request);
			case eAddMethod:
				HandleAddMethodRequest(request);
			case eQuery:
				result = HandleQueryRequest(request);
			case eUnknown:
				HandleUnknownRequest();
		}

		return result;

	}

	void RequestHandler::HandleAddProjectRequest(string request)
	{
		Project project = JsonToProject(request);
		database.AddProject(project);
		return;
	}

	void RequestHandler::HandleAddMethodRequest(string request)
	{
		Method method = JsonToMethod(request);
		Project project;
		database.AddMethod(method, project);
		return;
	}

	string RequestHandler::HandleQueryRequest(string request)
	{
		string hash = request.substr(request.find(" ") + 1);
		vector<Method> methods = database.HashToMethods(hash);
		if(methods.size() == 0)
			return "No results found";
		nlohmann::json result;
		for(int i = 0; i < methods.size(); i++){
		result["Method " + i] = nlohmann::json{{"hash", methods[i].hash}, {"name", methods[i].methodName}, {"file", methods[i].fileLocation}};
		}
		return result.dump();
	}

	void RequestHandler::HandleUnknownRequest()
	{
		cout << "Your request is not recognised." << endl;
		return;
	}

	Project RequestHandler::JsonToProject(string request)
	{
		// Todo: convert the project which is given in json format in the request to an actual project.
		Project project;
		return project;
	}

	Method RequestHandler::JsonToMethod(string request)
	{
		// Todo: convert the method which is given in json format in the request to an actual method.
		Method method;
		return method;
	}

	// Determines the type of the request.
	eRequestType RequestHandler::RequestToRequestType(string request)
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

/*enum eRequestType
{
	eAddProject,
	eAddMethod,
	eQuery,
	eUnknown
};*/

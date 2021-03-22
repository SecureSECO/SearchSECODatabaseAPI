/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include <iostream>
#include <string>
#include <tuple>
//#include <json.hpp>

#include "RequestHandler.h"

using namespace std;

/// <summary>
/// Handles requests towards database.
/// </summary>
// Initialise the RequestHandler
void RequestHandler::Initialize()
{
	// Set up a connection with the database.
    database.Connect();
}

string RequestHandler::HandleRequest(string request)
{
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

// Handles "add <Project>"-requests.
void RequestHandler::HandleAddProjectRequest(string request)
{
	nlohmann::json json = nlohmann::json::parse(request);
	Project project = JsonToProject(json);
	database.AddProject(project);
	return;
}

// Handles "add <Method>"-requests.
void RequestHandler::HandleAddMethodRequest(string request)
{
	nlohmann::json json = nlohmann::json::parse(request);
	tuple <MethodIn, ProjectID, Version> result = JsonToTuple(json);

	Project project;
	project.projectID = get<1>(result);
	project.version = get<2>(result);

	database.AddMethod(get<0>(result), project);
	return;
}

// Handles query requests.
string RequestHandler::HandleQueryRequest(string request)
{
	string hash = request.substr(request.find(" ") + 1);
	vector<MethodOut> methods = database.HashToMethods(hash);
	if(methods.size() == 0)
		return "No results found";
	else
	{
		nlohmann::json result;
		for (int i = 0; i < methods.size(); i++) {
			result["Method " + to_string(i)] = nlohmann::json{ {"hash", methods[i].hash}, {"name", methods[i].methodName}, {"file", methods[i].fileLocation}, {"authors", ToString(methods[i].authorIDs)} };
		}
		return result.dump();
	}
}

// Handles unknown requests.
void RequestHandler::HandleUnknownRequest()
{
	cout << "Your request is not recognised." << endl;
	return;
}

Project RequestHandler::JsonToProject(nlohmann::json json)
{
	Project project;
	project.projectID = json["projectID"];
	project.version = json["version"];
	project.license = json["license"];
	project.name = json["name"];
	project.url = json["url"];
	project.owner = JsonToAuthor(json["owner"]);
	project.stars = json["stars"];
	project.hashes = json["hashes"].get<vector<string>>();
	return project;
}

Author RequestHandler::JsonToAuthor(nlohmann::json json)
{
	Author author;
	author.name = json["name"];
	author.mail = json["mail"];
	return author;
}

vector<Author> RequestHandler::MapJsonToAuthor(vector<nlohmann::json> jsons)
{
	vector<Author> authors;
	for (int i = 0; i < jsons.size(); i++)
		authors.push_back(JsonToAuthor(jsons[i]));

	return authors;
}

tuple <MethodIn, ProjectID, Version> RequestHandler::JsonToTuple(nlohmann::json json)
{
	MethodIn method;
	method.hash = json["hash"];
	method.methodName = json["methodName"];
	method.fileLocation = json["fileLocation"];
	method.authors = MapJsonToAuthor(json["authors"]);
	
	ProjectID projectID = json["projectID"];
	Version version = json["version"];

	return make_tuple(method, projectID, version);
}

// Converts a vector of strings to a string with list formatting.
string RequestHandler::ToString(vector<string> values)
{
	if (values.empty())
		return "[]";
	else
	{
		string output = "[" + values[0];
		for (int i = 1; i < values.size(); i++)
			output += ", " + values[i];
		return output + "]";
	}
}

// Determines the type of the request.
eRequestType RequestHandler::RequestToRequestType(string request)
{
	// Get first word of request, which determines its type.
	string requestType = request.substr(0, request.find(" "));
	
	if (requestType == "addp")
		return eAddProject;
	else if (requestType == "addm")
		return eAddMethod;
	else if (requestType == "query")
		return eQuery;
	else return eUnknown;
}

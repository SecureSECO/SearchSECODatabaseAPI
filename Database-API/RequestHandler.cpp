/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include <iostream>
#include <string>
#include <tuple>
#include <sstream>

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

string RequestHandler::HandleRequest(string requestType, string request)
{
	// We determine the type of the request.
	eRequestType eRequestType = GetERequestType(requestType);

	request = request.substr(request.find(" ") + 1);
	//vector<vector<string>> data = requestToData(request);

	// We handle the request based on its type.
	string result;
	switch (eRequestType)
	{
		case eAddProject:
			HandleAddProjectRequest(request);
		case eQuery:
			result = HandleQueryRequest(request);
		case eUnknown:
			HandleUnknownRequest();
	}
	return result;
}

// Handles "add <Project>"-requests.
void RequestHandler::HandleAddProjectRequest(string request) // request = projectID|version|license|project_name|url|author_name|author_mail|stars \n method1_hash|method1_name|method1_fileLocation|method1_number_of_authors|method1_author1_name|method1_author1_mail|... \n ...
{
	Project project = RequestToProject(request);
	MethodIn method;

	vector<string> dataEntries = SplitStringOn(request, '\n');
	for (int i = 1; i < dataEntries.size(); i++)
	{
		method = DataEntryToMethod(dataEntries[i]);
		database.AddMethod(method, project);
	}
	return;
}

Project RequestHandler::RequestToProject(string request) // project = projectID|version|license|project_name|url|author_name|author_mail|stars
{
	// Convert request to projectData
	string project_string = request.substr(0, request.find('\n'));
	vector<string> projectData = SplitStringOn(project_string, '\0');

	Project project;
	project.projectID  = projectData[0];
	project.version    = projectData[1];
	project.license    = projectData[2];
	project.name       = projectData[3];
	project.url        = projectData[4];
	project.owner.name = projectData[5];
	project.owner.mail = projectData[6];
	project.stars = stoi(projectData[7]); // std::stoi converts a string to an int.
	project.hashes = RequestToHashes(request);
	return project;
}

MethodIn RequestHandler::DataEntryToMethod(string dataEntry) // methodData = method_hash|method_name|method_fileLocation|number_of_authors|method_author1_name|method_author1_mail|method_author2_name|method_author2_mail|...
{
	vector<string> methodData = SplitStringOn(dataEntry, '\0');

	MethodIn method;
	method.hash = methodData[0];
	method.methodName = methodData[1];
	method.fileLocation = methodData[2];

	vector<Author> authors;
	Author author;
	int numberOfAuthors = stoi(methodData[3]);
	for (int i = 0; i < numberOfAuthors; i++)
	{
		author.name = methodData[4 + 2 * i];
		author.mail = methodData[5 + 2 * i];

		authors.push_back(author);
	}
	method.authors = authors;
	return method;
}

vector<Hash> RequestHandler::RequestToHashes(string request)
{
	// TODO: Find all the hashes inside the request.
	return {};
}

// Handles query requests.
string RequestHandler::HandleQueryRequest(string request) // request = hash; output = method1_hash|method1_name|method1_fileLocation|number_of_authors|method1_author1_name|method1_author1_mail|method1_author2_name|method1_author2_mail|... \n <method2_data> \n ...
{
	string hash = request.substr(0, request.find(" "));
	vector<MethodOut> methods = database.HashToMethods(hash);
	if(!methods.empty())
	{
		string result;
		for (int i = 0; i < methods.size(); i++) 
		{
			result += MethodToString(methods[i]) + "\n"; // TODO: Get result efficiently, probably with use of vector<char> instead of string.
		}
		return result;
	}
	else return "No results found";
}

string RequestHandler::MethodToString(MethodOut method)
{
	string result;
	string hash = method.hash;
	string name = method.methodName;
	string fileLocation = method.fileLocation;
	vector<string> authorids = method.authorIDs;
	string authorTotal = to_string(authorids.size());

	result = ""; // TODO: Get result efficiently, maybe with use of vector<char> instead of string.
	return result;
}

vector<string> RequestHandler::SplitStringOn(string str, char delimiter)
{
	stringstream strstream(str);
	string item;
	vector<string> substrings;
	while (getline(strstream, item, delimiter))
	{
		substrings.push_back(item);
	}
	return substrings;
}

// Handles unknown requests.
void RequestHandler::HandleUnknownRequest()
{
	cout << "Your request is not recognised." << endl;
	return;
}

// Converts a vector of strings to a string with spaces between entries
// Example: ["abc", "def", "ghi"] -> "abc def ghi". 
string RequestHandler::ToString(vector<string> values)
{
	if (!values.empty())
	{
		string result = values[0];
		for (int i = 1; i < values.size(); i++)
			result += " " + values[i];
		return result;
	}
	else return "";
}

// Returns a vector which splits with respect to spaces.
// Example: "abc def ghi" -> ["abc", "def", "ghi"]
vector<string> RequestHandler::ToVector(string values)
{
	vector<string> result = SplitStringOn(values, ' ');
	return result;
}

// Determines the type of the request.
eRequestType RequestHandler::GetERequestType(string requestType)
{
	if (requestType == "addp")
		return eAddProject;
	else if (requestType == "quer")
		return eQuery;
	else return eUnknown;
}

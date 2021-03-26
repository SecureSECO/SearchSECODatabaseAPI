/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <ctime>

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
	// We convert the requestType to a eRequestType (for the switch below).
	eRequestType eRequestType = GetERequestType(requestType);

	// We handle the request based on its type.
	string result;
	switch (eRequestType)
	{
		case eUpload:
			result = HandleUploadRequest(request);
			break;
		case eCheck:
			result = HandleCheckRequest(request);
			break;
		case eCheckUpload:
			result = HandleCheckUploadRequest(request);
		case eUnknown:
			result = HandleUnknownRequest();
			break;
	}
	return result;
}

string RequestHandler::HandleCheckUploadRequest(string request) // request = projectID|version|license|project_name|url|author_name|author_mail|stars \n method1_hash|method1_name|method1_fileLocation|method1_lineNumber|method1_number_of_authors|method1_author1_name|method1_author1_mail|... \n ...
{
	vector<Hash> hashes = RequestToHashes(request);
	string result = HandleCheckRequest(hashes);
	HandleUploadRequest(request);
	return result;
}

// Handles "add <Project>"-requests.
string RequestHandler::HandleUploadRequest(string request) // request = projectID|version|license|project_name|url|author_name|author_mail|stars \n method1_hash|method1_name|method1_fileLocation|method1_lineNumber|method1_number_of_authors|method1_author1_name|method1_author1_mail|... \n ...
{
	Project project = RequestToProject(request);
	MethodIn method;

	vector<string> dataEntries = SplitStringOn(request, '\n');
	for (int i = 1; i < dataEntries.size(); i++)
	{
		method = DataEntryToMethod(dataEntries[i]);
		database.AddMethod(method, project);
	}
	return "Your project is successfully added to the database.";
}

// Retrieves the project given a request.
Project RequestHandler::RequestToProject(string request) // project = projectID|version|license|project_name|url|author_name|author_mail|stars
{
	// Convert request to projectData
	string project_string = request.substr(0, request.find('\n'));
	vector<string> projectData = SplitStringOn(project_string, '?');

	Project project;
	project.projectID  = projectData[0];
	project.version    = stoll(projectData[1]); // std::stoll converts a string to a long int.
	project.license    = projectData[2];
	project.name       = projectData[3];
	project.url        = projectData[4];
	project.owner.name = projectData[5];
	project.owner.mail = projectData[6];
	project.stars      = stoi(projectData[7]); // std::stoi converts a string to an int.
	project.hashes     = RequestToHashes(request);
	return project;
}

// Converts a data entry to a Method.
MethodIn RequestHandler::DataEntryToMethod(string dataEntry) // methodData = method_hash|method_name|method_fileLocation|number_of_authors|method_author1_name|method_author1_mail|method_author2_name|method_author2_mail|...
{
	vector<string> methodData = SplitStringOn(dataEntry, '?');

	MethodIn method;
	method.hash = methodData[0];
	method.methodName = methodData[1];
	method.fileLocation = methodData[2];
	method.lineNumber = stoi(methodData[3]);
	vector<Author> authors;
	Author author;
	int numberOfAuthors = stoi(methodData[4]);
	for (int i = 0; i < numberOfAuthors; i++)
	{
		author.name = methodData[5 + 2 * i];
		author.mail = methodData[6 + 2 * i];

		authors.push_back(author);
	}
	method.authors = authors;
	return method;
}

// Retrieves the hashes within a request.
vector<Hash> RequestHandler::RequestToHashes(string request)
{
	vector<string> data = SplitStringOn(request, '\n');
	vector<Hash> hashes = {};
	for (int i = 1; i < data.size(); i++)
	{
		Hash hash = data[i].substr(0, data[i].find('?'));
		hashes.push_back(hash);
	}
	return hashes;
}

// Handles requests (consisting of hashes separated by newline characters) by returning methods (in string format) with the same hash.
string RequestHandler::HandleCheckRequest(string request) // request = hash1 \n hash2 \n ...;  output = method1_hash|method1_name|method1_fileLocation|method1_lineNumber|number_of_authors|method1_authorid1|method1_authorid2|... \n <method2_data> \n ...
{
	vector<Hash> hashes = SplitStringOn(request, '\n');
	return HandleCheckRequest(hashes);
}

string RequestHandler::HandleCheckRequest(vector<Hash> hashes)
{
	vector<MethodOut> methods = GetMethods(hashes);

	string methodsToString = MethodsToString(methods, '?', '\n');
	if (!(methodsToString == ""))
	{
		return methodsToString;
	}
	else return "No results found";
}

// Retrieves the methods corresponding to the hashes given as input using the database.
vector<MethodOut> RequestHandler::GetMethods(vector<Hash> hashes)
{
	vector<MethodOut> methods = { };
	for (int i = 0; i < hashes.size(); i++)
	{
		vector<MethodOut> newMethods = database.HashToMethods(hashes[i]);
		for (int j = 0; j < newMethods.size(); j++)
		{
			methods.push_back(newMethods[j]);
		}
	}
	return methods;
}

// Appends a vector of chars 'result' by methods which still need to be converted to vectors of chars. Also separates different methods and different method data elements by special characters.
string RequestHandler::MethodsToString(vector<MethodOut> methods, char dataDelimiter, char methodDelimiter)
{
	vector<char> chars = {};
	while (!methods.empty())
	{
		MethodOut lastMethod     = methods.back();
		string hash              = lastMethod.hash;
		string projectID         = lastMethod.projectID;
		string version           = to_string(lastMethod.version);
		string name              = lastMethod.methodName;
		string fileLocation      = lastMethod.fileLocation;
		string lineNumber        = to_string(lastMethod.lineNumber);
		vector<string> authorIDs = lastMethod.authorIDs;
		string authorTotal       = to_string(authorIDs.size());

		// We initialize dataElements, which consists of the hash, projectID, version, name, fileLocation, lineNumber, authorTotal and all the authorIDs.
		vector<string> dataElements = { hash, projectID, version, name, fileLocation, lineNumber, authorTotal };
		dataElements.insert(end(dataElements), begin(authorIDs), end(authorIDs));

		for (string data : dataElements)
		{
			AppendBy(chars, data, dataDelimiter);
		}

		// We should get rid of the last dataDelimiter if something is appended to 'chars' (which is only the case if it is non-empty).
		if (!chars.empty())
		{
			chars.pop_back();
		}

		// We end 'chars' with the methodDelimiter and indicate that we are done with 'method'.
		chars.push_back(methodDelimiter);
		methods.pop_back();
	}
	string result(chars.begin(), chars.end()); // Converts the vector of chars to a string.
	return result;
}

// Appends result-string by a string, and adds a special character at the end.
void RequestHandler::AppendBy(vector<char>& result, string word, char endCharacter)
{
	for (int i = 0; i < word.size(); i++)
	{
		result.push_back(word[i]);
	}
	result.push_back(endCharacter);
}

// Splits a string on a special character and returns the vectors consisting of the substrings.
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
string RequestHandler::HandleUnknownRequest()
{
	return "Your input is not recognised.";
}

// Determines the type of the request.
eRequestType RequestHandler::GetERequestType(string requestType)
{
	if (requestType == "upld")
		return eUpload;
	else if (requestType == "chck")
		return eCheck;
	else if (requestType == "chup")
		return eCheckUpload;
	else return eUnknown;
}

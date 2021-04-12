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

void RequestHandler::initialize(DatabaseHandler *databaseHandler)
{
	// Set up a connection with the database.
	database = databaseHandler;
	database -> connect();
}

string RequestHandler::handleRequest(string requestType, string request)
{
	// We convert the requestType to a eRequestType (for the switch below).
	eRequestType eRequestType = getERequestType(requestType);

	// We handle the request based on its type.
	string result;
	switch (eRequestType)
	{
		case eUpload:
			result = handleUploadRequest(request);
			break;
		case eCheck:
			result = handleCheckRequest(request);
			break;
		case eCheckUpload:
			result = handleCheckUploadRequest(request);
			break;
		case eUnknown:
			result = handleUnknownRequest();
			break;
	}
	return result;
}

// Handles requests wanting to first check for matches with existing entries,
// after which it adds the project itself to the database.
// In this case, the request has the following format:
// "projectID?version?license?project_name?url?author_name?author_mail?stars \n
//  method1_hash?method1_name?method1_fileLocation?method1_lineNumber?method1_numberOfAuthors?
//  method1_author1_name?method1_author1_mail? <other authors> \n <method2_data> \n ... <methodN_data>".
string RequestHandler::handleCheckUploadRequest(string request)
{
	vector<Hash> hashes = requestToHashes(request);
	string result = handleCheckRequest(hashes);
	handleUploadRequest(request);
	return result;
}

vector<Hash> RequestHandler::requestToHashes(string request)
{
	vector<string> data = splitStringOn(request, '\n');
	vector<Hash> hashes = {};
	for (int i = 1; i < data.size(); i++)
	{
		Hash hash = data[i].substr(0, data[i].find('?'));
		hashes.push_back(hash);
	}
	return hashes;
}

// Handles requests which simply want to add a project to the database.
// The request has the same format as mentioned previously.
string RequestHandler::handleUploadRequest(string request)
{
	Project project = requestToProject(request);
	database -> addProject(project);
	MethodIn method;

	vector<string> dataEntries = splitStringOn(request, '\n');

	for (int i = 1; i < dataEntries.size(); i++)
	{
		method = dataEntryToMethod(dataEntries[i]);

		database -> addMethod(method, project);
	}
	return "Your project is successfully added to the database.";
}

Project RequestHandler::requestToProject(string request)
{
	// We retrieve the project information (projectData).
	string projectString = request.substr(0, request.find('\n'));
	vector<string> projectData = splitStringOn(projectString, '?');

	// We return the project information in the form of a Project.
	Project project;
	project.projectID  = stoll(projectData[0]);
	project.version    = stoll(projectData[1]); // std::stoll converts a string to a long int.
	project.license    = projectData[2];
	project.name       = projectData[3];
	project.url        = projectData[4];
	project.owner.name = projectData[5];
	project.owner.mail = projectData[6];

	return project;
}

MethodIn RequestHandler::dataEntryToMethod(string dataEntry)
{
	vector<string> methodData = splitStringOn(dataEntry, '?');

	MethodIn method;
	method.hash = methodData[0];
	method.methodName = methodData[1];
	method.fileLocation = methodData[2];
	method.lineNumber = stoi(methodData[3]);

	vector<Author> authors;
	int numberOfAuthors = stoi(methodData[4]);
	for (int i = 0; i < numberOfAuthors; i++)
	{
		Author author;
		author.name = methodData[5 + 2 * i];
		author.mail = methodData[6 + 2 * i];
		authors.push_back(author);
	}
	method.authors = authors;

	return method;
}

// Handles requests (consisting of hashes separated by newline characters) by returning methods (in string format) with
// the same hash. In this case, the request has the following format:
// "hash_1\nhash_2\n...\nhash_N".
// The output has the following format:
// "method1_hash|method1_name|method1_fileLocation|method1_lineNumber|number_of_authors|method1_authorid1|...|
//  method1_authoridM\n<method2_data>\n ... \n<methodN_data>".
string RequestHandler::handleCheckRequest(string request)
{
	vector<Hash> hashes = splitStringOn(request, '\n');
	return handleCheckRequest(hashes);
}

string RequestHandler::handleCheckRequest(vector<Hash> hashes)
{
	vector<MethodOut> methods = getMethods(hashes);

	string methodsStringFormat = methodsToString(methods, '?', '\n');
	if (!(methodsStringFormat == ""))
	{
		return methodsStringFormat;
	}
	else
	{
		return "No results found";
	}
}

// Retrieves the methods corresponding to the hashes given as input using the database.
vector<MethodOut> RequestHandler::getMethods(vector<Hash> hashes)
{
	vector<MethodOut> methods = { };
	for (int i = 0; i < hashes.size(); i++)
	{
		vector<MethodOut> newMethods = database -> hashToMethods(hashes[i]);

		for (int j = 0; j < newMethods.size(); j++)
		{
			methods.push_back(newMethods[j]);
		}
	}
	return methods;
}

// Appends a vector of chars 'result' by methods which still need to be converted to vectors of chars. Also separates
// different methods and different method data elements by special characters.
string RequestHandler::methodsToString(vector<MethodOut> methods, char dataDelimiter, char methodDelimiter)
{
	vector<char> chars = {};
	while (!methods.empty())
	{
		MethodOut lastMethod     = methods.back();
		string hash              = lastMethod.hash;
		string projectID         = to_string(lastMethod.projectID);
		string version           = to_string(lastMethod.version);
		string name              = lastMethod.methodName;
		string fileLocation      = lastMethod.fileLocation;
		string lineNumber        = to_string(lastMethod.lineNumber);
		vector<string> authorIDs = lastMethod.authorIDs;
		string authorTotal       = to_string(authorIDs.size());

		// We initialize dataElements, which consists of the hash, projectID, version, name, fileLocation, lineNumber,
		// authorTotal and all the authorIDs.
		vector<string> dataElements = { hash, projectID, version, name, fileLocation, lineNumber, authorTotal };
		dataElements.insert(end(dataElements), begin(authorIDs), end(authorIDs));

		for (string data : dataElements)
		{
			appendBy(chars, data, dataDelimiter);
		}

		// We should get rid of the last dataDelimiter if something is appended to 'chars',
		// which is precisely the case when it is non-empty.
		if (!chars.empty())
		{
			chars.pop_back();
		}

		// We end 'chars' with the methodDelimiter and indicate that we are done with 'lastMethod'.
		chars.push_back(methodDelimiter);
		methods.pop_back();
	}
	string result(chars.begin(), chars.end()); // Converts the vector of chars to a string.
	return result;
}

// Appends result-string by a string, and adds a special character at the end.
void RequestHandler::appendBy(vector<char>& result, string word, char endCharacter)
{
	for (int i = 0; i < word.size(); i++)
	{
		result.push_back(word[i]);
	}
	result.push_back(endCharacter);
}

vector<string> RequestHandler::splitStringOn(string str, char delimiter)
{
	stringstream strStream(str);
	string item;
	vector<string> substrings;
	while (getline(strStream, item, delimiter))
	{
		substrings.push_back(item);
	}
	return substrings;
}

// Handles unknown requests.
string RequestHandler::handleUnknownRequest()
{
	return "Your input is not recognised.";
}

// Converts a requestType into an eRequestType which can be used inside a switch-statement.
eRequestType RequestHandler::getERequestType(string requestType)
{
	if (requestType == "upld")
	{
		return eUpload;
	}
	else if (requestType == "chck")
	{
		return eCheck;
	}
	else if (requestType == "chup")
	{
		return eCheckUpload;
	}
	else
	{
		return eUnknown;
	}
}

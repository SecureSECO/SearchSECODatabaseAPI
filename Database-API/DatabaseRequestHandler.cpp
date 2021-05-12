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
#include <regex>

#include "DatabaseRequestHandler.h"
#include "Utility.h"

using namespace std;

DatabaseRequestHandler::DatabaseRequestHandler(DatabaseHandler *database, std::string ip, int port) 
{
	this->database = database;
	database -> connect(ip, port);
}

string DatabaseRequestHandler::handleCheckUploadRequest(string request)
{
	vector<Hash> hashes = requestToHashes(request);
	if (errno != 0)
	{
		// Hashes could not be parsed.
		return "Error parsing hashes.";
	}
	string result = handleCheckRequest(hashes);
	handleUploadRequest(request);
	return result;
}

vector<Hash> DatabaseRequestHandler::requestToHashes(string request)
{
	errno = 0;
	vector<string> data = Utility::splitStringOn(request, '\n');
	vector<Hash> hashes = {};
	for (int i = 1; i < data.size(); i++)
	{
		// Data before first delimiter.
		Hash hash = data[i].substr(0, data[i].find('?'));
		if (isValidHash(hash))
		{
			hashes.push_back(hash);
		}
		else
		{
			// Invalid hash in sequence.
			errno = EILSEQ;
			return vector<Hash>();
		}
	}
	return hashes;
}

bool DatabaseRequestHandler::isValidHash(Hash hash)
{
	// Inspired by: https://stackoverflow.com/questions/19737727/c-check-if-string-is-a-valid-md5-hex-hash.
	return hash.size() == 32 && hash.find_first_not_of(HEX_CHARS) == -1;
}

string DatabaseRequestHandler::handleUploadRequest(string request)
{
	// Check if project is valid.
	Project project = requestToProject(request);
	if (errno != 0)
	{
		// Project could not be parsed.
		return "Error parsing project data.";
	}

	vector<MethodIn> methods;

	vector<string> dataEntries = Utility::splitStringOn(request, '\n');

	// Check if all methods are valid.
	for (int i = 1; i < dataEntries.size(); i++)
	{
		MethodIn method = dataEntryToMethod(dataEntries[i]);
		if (errno != 0)
		{
			return "Error parsing method " + std::to_string(i) + ".";
		}
		methods.push_back(method);
	}

	// Only upload if project and all methods are valid to prevent partial uploads.
	database -> addProject(project);
	for (MethodIn method : methods)
	{
		database -> addMethod(method, project);
	}
	if (errno == 0)
	{
		return "Your project has been successfully added to the database.";
	}
	else
	{
		return "An unexpected error occurred.";
	}
}

Project DatabaseRequestHandler::requestToProject(string request)
{
	errno = 0;
	// We retrieve the project information (projectData).
	string projectString = request.substr(0, request.find('\n'));
	vector<string> projectData = Utility::splitStringOn(projectString, '?');

	if (projectData.size() != PROJECT_DATA_SIZE)
	{
		errno = EILSEQ;
		return Project();
	}

	// We return the project information in the form of a Project.
	Project project;
	project.projectID  = Utility::safeStoll(projectData[0]);	// Converts string to long long.
	if (errno != 0)
	{
		return Project();
	}
	project.version    = Utility::safeStoll(projectData[1]);
	if (errno != 0)
	{
		return Project();
	}
	project.license    = projectData[2];
	project.name       = projectData[3];
	project.url        = projectData[4];
	project.owner.name = projectData[5];
	project.owner.mail = projectData[6];

	return project;
}

MethodIn DatabaseRequestHandler::dataEntryToMethod(string dataEntry)
{
	errno = 0;
	vector<string> methodData = Utility::splitStringOn(dataEntry, '?');

	if (methodData.size() < METHOD_DATA_MIN_SIZE)
	{
		// Too few parameters.
		errno = EILSEQ;
		return MethodIn();
	}

	MethodIn method;
	if (!isValidHash(methodData[0]))
	{
		// Invalid method hash.
		errno = EILSEQ;
		return MethodIn();
	}
	method.hash = methodData[0];
	method.methodName = methodData[1];
	method.fileLocation = methodData[2];
	method.lineNumber = Utility::safeStoi(methodData[3]);
	if (errno != 0)
	{
		// Non-integer line number.
		return MethodIn();
	}

	vector<Author> authors;
	int numberOfAuthors = Utility::safeStoi(methodData[4]);
	if (errno != 0)
	{
		// Non-integer number of authors.
		return MethodIn();
	}

	if (methodData.size() != METHOD_DATA_MIN_SIZE + 2 * numberOfAuthors)
	{
		// Incorrect amount of parameters.
		errno = EILSEQ;
		return MethodIn();
	}

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

string DatabaseRequestHandler::handleCheckRequest(string request)
{
	vector<Hash> hashes = Utility::splitStringOn(request, '\n');
	return handleCheckRequest(hashes);
}

string DatabaseRequestHandler::handleCheckRequest(vector<Hash> hashes)
{
	// Check if all requested hashes are invalid.
	for (int i = 0; i < hashes.size(); i++)
	{
		if (!isValidHash(hashes[i]))
		{
			return "Invalid hash presented.";
		}
	}

	// Request the specified hashes.
	vector<MethodOut> methods = getMethods(hashes);

	// Return retrieved data.
	string methodsStringFormat = methodsToString(methods, '?', '\n');
	if (!(methodsStringFormat == ""))
	{
		return methodsStringFormat;
	}
	else
	{
		return "No results found.";
	}
}

vector<MethodOut> DatabaseRequestHandler::getMethods(vector<Hash> hashes)
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

string DatabaseRequestHandler::methodsToString(vector<MethodOut> methods, char dataDelimiter, char methodDelimiter)
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
			Utility::appendBy(chars, data, dataDelimiter);
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

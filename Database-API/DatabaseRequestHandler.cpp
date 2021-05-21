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
#include <thread>
#include <future>
#include <functional>

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

	queue<MethodIn> methodQueue;
	mutex queueLock;
	vector<thread> threads;

	for (MethodIn method : methods)
	{
		methodQueue.push(method);
	}
	for (int i = 0; i < MAX_THREADS; i++)
	{
		threads.push_back(thread(&DatabaseRequestHandler::singleUploadThread, this, ref(methodQueue), ref(queueLock), project));
	}
	for (int i = 0; i < threads.size(); i++)
	{
		threads[i].join();
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

void DatabaseRequestHandler::singleUploadThread(queue<MethodIn> &methods, mutex &queueLock, Project project)
{
	while (true)
	{
		queueLock.lock();
		if (methods.size() <= 0)
		{
			queueLock.unlock();
			return;
		}
		MethodIn method = methods.front();
		methods.pop();
		queueLock.unlock();
		database->addMethod(method, project);
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
	vector<future<vector<MethodOut>>> results;
	vector<thread> threads;
	queue<Hash> hashQueue;
	mutex queueLock;
	for (int i = 0; i < hashes.size(); i++)
	{
		hashQueue.push(hashes[i]);
	}
	for (int i = 0; i < MAX_THREADS; i++)
	{
		packaged_task<vector<MethodOut>()> task(
			bind(&DatabaseRequestHandler::singleHashToMethodsThread, this, ref(hashQueue), ref(queueLock)));
		results.push_back(task.get_future());
		threads.push_back(thread(move(task)));
	}
	for (int i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}
	vector<MethodOut> methods = {};
	for (int i = 0; i < results.size(); i++)
	{
		vector<MethodOut> newMethods = results[i].get();

		for (int j = 0; j < newMethods.size(); j++)
		{
			methods.push_back(newMethods[j]);
		}
	}
	return methods;
}

vector<MethodOut> DatabaseRequestHandler::singleHashToMethodsThread(queue<Hash> &hashes, mutex &queueLock)
{
	vector<MethodOut> methods;
	while (true)
	{
		queueLock.lock();
		if (hashes.size() <= 0)
		{
			queueLock.unlock();
			return methods;
		}
		Hash hash = hashes.front();
		hashes.pop();
		queueLock.unlock();
		vector<MethodOut> newMethods = database->hashToMethods(hash);
		for (int j = 0; j < newMethods.size(); j++)
		{
			methods.push_back(newMethods[j]);
		}
	}
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

string DatabaseRequestHandler::handleGetAuthorIDRequest(string request)
{
	errno = 0;
	vector<string> authorStrings = Utility::splitStringOn(request, '\n');

	vector<Author> authors;

	for (int i = 0; i < authorStrings.size(); i++)
	{
		authors.push_back(datanEntryToAuthor(authorStrings[i]));
		if (errno != 0)
		{
			return "Error parsing author: " + authorStrings[i];
		}
	}

	// Request the specified hashes.
	vector<tuple<Author, string>> authorIDs = getAuthorIDs(authors);

	if (authorIDs.size() <= 0)
	{
		return "No results found.";
	}

	return authorsToString(authorIDs);
}

string DatabaseRequestHandler::authorsToString(vector<tuple<Author, string>> authors)
{
	vector<char> chars = {};
	while (!authors.empty())
	{
		tuple<Author, string> lastAuthorID = authors.back();
		string name = get<0>(lastAuthorID).name;
		string mail = get<0>(lastAuthorID).mail;
		string id = get<1>(lastAuthorID);

		// We initialize dataElements, which consists of the hash, projectID, version, name, fileLocation, lineNumber,
		// authorTotal and all the authorIDs.
		vector<string> dataElements = {name, mail, id};

		for (string data : dataElements)
		{
			Utility::appendBy(chars, data, '?');
		}

		// We should get rid of the last dataDelimiter if something is appended to 'chars',
		// which is precisely the case when it is non-empty.
		if (!chars.empty())
		{
			chars.pop_back();
		}

		// We end 'chars' with the methodDelimiter and indicate that we are done with 'lastMethod'.
		chars.push_back('\n');
		authors.pop_back();
	}
	string result(chars.begin(), chars.end()); // Converts the vector of chars to a string.
	return result;
}

Author DatabaseRequestHandler::datanEntryToAuthor(string dataEntry)
{
	vector<string> authorData = Utility::splitStringOn(dataEntry, '?');

	Author author;

	if (authorData.size() != 2)
	{
		errno = EILSEQ;
		return author;
	}
	
	author.name = authorData[0];
	author.mail = authorData[1];

	return author;
}

vector<tuple<Author, string>> DatabaseRequestHandler::getAuthorIDs(vector<Author> authors)
{
	vector<future<vector<tuple<Author, string>>>> results;
	vector<thread> threads;
	queue<Author> authorQueue;
	mutex queueLock;
	for (int i = 0; i < authors.size(); i++)
	{
		authorQueue.push(authors[i]);
	}
	for (int i = 0; i < MAX_THREADS; i++)
	{
		packaged_task<vector<tuple<Author, string>>()> task(
			bind(&DatabaseRequestHandler::singleAuthorToIDThread, this, ref(authorQueue), ref(queueLock)));
		results.push_back(task.get_future());
		threads.push_back(thread(move(task)));
	}
	for (int i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}
	vector<tuple<Author, string>> authorIDs;
	for (int i = 0; i < results.size(); i++)
	{
		vector<tuple<Author, string>> newAuthorIDs = results[i].get();

		for (int j = 0; j < newAuthorIDs.size(); j++)
		{
			authorIDs.push_back(newAuthorIDs[j]);
		}
	}
	return authorIDs;
}

vector<tuple<Author, string>> DatabaseRequestHandler::singleAuthorToIDThread(queue<Author> &authors, mutex &queueLock)
{
	vector<tuple<Author, string>> authorIDs;
	while (true)
	{
		queueLock.lock();
		if (authors.size() <= 0)
		{
			queueLock.unlock();
			return authorIDs;
		}
		Author author = authors.front();
		authors.pop();
		queueLock.unlock();
		string newAuthorID = database->authorToId(author);
		if (newAuthorID != "")
		{
			authorIDs.push_back(make_tuple(author, newAuthorID));
		}
	}
}

string DatabaseRequestHandler::handleGetAuthorRequest(string request)
{
	vector<string> authorIds = Utility::splitStringOn(request, '\n');

	regex re(UUID_REGEX);

	for (int i = 0; i < authorIds.size(); i++)
	{
		if (!regex_match(authorIds[i], re))
		{
			return "Error parsing author id: " + authorIds[i];
		}
	}

	// Request the specified hashes.
	vector<tuple<Author, string>> authors = getAuthors(authorIds);

	if (authors.size() <= 0)
	{
		return "No results found.";
	}

	return authorsToString(authors);
}

vector<tuple<Author, string>> DatabaseRequestHandler::getAuthors(vector<string> authorIds)
{
	vector<future<vector<tuple<Author, string>>>> results;
	vector<thread> threads;
	queue<string> authorIdQueue;
	mutex queueLock;
	for (int i = 0; i < authorIds.size(); i++)
	{
		authorIdQueue.push(authorIds[i]);
	}
	for (int i = 0; i < MAX_THREADS; i++)
	{
		packaged_task<vector<tuple<Author, string>>()> task(
			bind(&DatabaseRequestHandler::singleIdToAuthorThread, this, ref(authorIdQueue), ref(queueLock)));
		results.push_back(task.get_future());
		threads.push_back(thread(move(task)));
	}
	for (int i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}
	vector<tuple<Author, string>> authors;
	for (int i = 0; i < results.size(); i++)
	{
		vector<tuple<Author, string>> newAuthors = results[i].get();

		for (int j = 0; j < newAuthors.size(); j++)
		{
			authors.push_back(newAuthors[j]);
		}
	}
	return authors;
}

vector<tuple<Author, string>> DatabaseRequestHandler::singleIdToAuthorThread(queue<string> &authorIds, mutex &queueLock)
{
	vector<tuple<Author, string>> authors;
	while (true)
	{
		queueLock.lock();
		if (authorIds.size() <= 0)
		{
			queueLock.unlock();
			return authors;
		}
		string id = authorIds.front();
		authorIds.pop();
		queueLock.unlock();
		Author newAuthor = database->idToAuthor(id);
		if (newAuthor.name != "" && newAuthor.mail != "")
		{
			authors.push_back(make_tuple(newAuthor, id));
		}
	}
}

string DatabaseRequestHandler::handleGetMethodsByAuthorRequest(string request)
{
	vector<string> authorIds = Utility::splitStringOn(request, '\n');

	regex re(UUID_REGEX);

	for (int i = 0; i < authorIds.size(); i++)
	{
		if (!regex_match(authorIds[i], re))
		{
			return "Error parsing author id: " + authorIds[i];
		}
	}

	// Request the specified hashes.
	vector<tuple<MethodId,string>> methods = getMethodsByAuthor(authorIds);

	// Return retrieved data.
	string methodsStringFormat = methodIdsToString(methods);
	if (!(methodsStringFormat == ""))
	{
		return methodsStringFormat;
	}
	else
	{
		return "No results found.";
	}
}

vector<tuple<MethodId, string>> DatabaseRequestHandler::getMethodsByAuthor(vector<string> authorIds)
{
	vector<future<vector<tuple<MethodId, string>>>> results;
	vector<thread> threads;
	queue<string> idQueue;
	mutex queueLock;
	for (int i = 0; i < authorIds.size(); i++)
	{
		idQueue.push(authorIds[i]);
	}
	for (int i = 0; i < MAX_THREADS; i++)
	{
		packaged_task<vector<tuple<MethodId, string>>()> task(
			bind(&DatabaseRequestHandler::singleAuthorToMethodsThread, this, ref(idQueue), ref(queueLock)));
		results.push_back(task.get_future());
		threads.push_back(thread(move(task)));
	}
	for (int i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}
	vector<tuple<MethodId, string>> methods = {};
	for (int i = 0; i < results.size(); i++)
	{
		vector<tuple<MethodId, string>> newMethods = results[i].get();

		for (int j = 0; j < newMethods.size(); j++)
		{
			methods.push_back(newMethods[j]);
		}
	}
	return methods;
}

vector<tuple<MethodId, string>> DatabaseRequestHandler::singleAuthorToMethodsThread(queue<string> &authorIds, mutex &queueLock)
{
	vector<tuple<MethodId, string>> methods;
	while (true)
	{
		queueLock.lock();
		if (authorIds.size() <= 0)
		{
			queueLock.unlock();
			return methods;
		}
		string authorId = authorIds.front();
		authorIds.pop();
		queueLock.unlock();
		vector<MethodId> newMethods = database->authorToMethods(authorId);
		for (int j = 0; j < newMethods.size(); j++)
		{
			methods.push_back(make_tuple(newMethods[j], authorId));
		}
	}
}

string DatabaseRequestHandler::methodIdsToString(vector<tuple<MethodId, string>> methods)
{
	vector<char> chars = {};
	while (!methods.empty())
	{
		tuple<MethodId, string> lastMethod = methods.back();
		string hash = get<0>(lastMethod).hash;
		string projectID = to_string(get<0>(lastMethod).projectId);
		string version = to_string(get<0>(lastMethod).version);
		string authorId = get<1>(lastMethod);

		// We initialize dataElements, which consists of the hash, projectID, version, name, fileLocation, lineNumber,
		// authorTotal and all the authorIDs.
		vector<string> dataElements = {authorId, hash, projectID, version};

		for (string data : dataElements)
		{
			Utility::appendBy(chars, data, '?');
		}

		// We should get rid of the last dataDelimiter if something is appended to 'chars',
		// which is precisely the case when it is non-empty.
		if (!chars.empty())
		{
			chars.pop_back();
		}

		// We end 'chars' with the methodDelimiter and indicate that we are done with 'lastMethod'.
		chars.push_back('\n');
		methods.pop_back();
	}
	string result(chars.begin(), chars.end()); // Converts the vector of chars to a string.
	return result;
}
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
#include <utility>
#include <functional>

#include "DatabaseRequestHandler.h"
#include "HTTPStatus.h"
#include "Utility.h"

DatabaseRequestHandler::DatabaseRequestHandler(DatabaseHandler *database, std::string ip, int port)
{
	this->database = database;
	database -> connect(ip, port);
}

std::string DatabaseRequestHandler::handleCheckUploadRequest(std::string request)
{
	std::vector<Hash> hashes = requestToHashes(request);
	if (errno != 0)
	{
		// Hashes could not be parsed.
		return HTTPStatusCodes::clientError("Error parsing hashes.");
	}
	std::string checkResult = handleCheckRequest(hashes);
	std::string checkStatusCode = HTTPStatusCodes::getCode(checkResult);

	// Check failed.
	if (checkStatusCode == "400")
	{
		return checkResult;
	}
	// Unknown response.
	else if (checkStatusCode != "200")
	{
		return checkResult;
	}

	std::string uploadResult = handleUploadRequest(request);
	std::string uploadStatusCode = HTTPStatusCodes::getCode(uploadResult);

	// Uploaded succeeded.
	if (uploadStatusCode == "200")
	{
		return checkResult;
	}
	// Check failed.
	else if (uploadStatusCode == "400")
	{
		return uploadResult;
	}
	// Unknown response.
	else
	{
		return uploadResult;
	}
}

std::vector<Hash> DatabaseRequestHandler::requestToHashes(std::string request)
{
	errno = 0;
	std::vector<std::string> data = Utility::splitStringOn(request, ENTRY_DELIMITER_CHAR);
	std::vector<Hash> hashes = {};
	for (int i = 1; i < data.size(); i++)
	{
		// Data before first delimiter.
		Hash hash = data[i].substr(0, data[i].find());
		if (isValidHash(hash))
		{
			hashes.push_back(hash);
		}
		else
		{
			// Invalid hash in sequence.
			errno = EILSEQ;
			return std::vector<Hash>();
		}
	}
	return hashes;
}

bool DatabaseRequestHandler::isValidHash(Hash hash)
{
	// Inspired by: https://stackoverflow.com/questions/19737727/c-check-if-string-is-a-valid-md5-hex-hash.
	return hash.size() == 32 && hash.find_first_not_of(HEX_CHARS) == -1;
}

std::string DatabaseRequestHandler::handleUploadRequest(std::string request)
{
	// Check if project is valid.
	ProjectIn project = requestToProject(request);
	if (errno != 0)
	{
		// Project could not be parsed.
		return HTTPStatusCodes::clientError("Error parsing project data.");
	}

	std::vector<MethodIn> methods;

	std::vector<std::string> dataEntries = Utility::splitStringOn(request, ENTRY_DELIMITER_CHAR);

	// Check if all methods are valid.
	for (int i = 1; i < dataEntries.size(); i++)
	{
		MethodIn method = dataEntryToMethod(dataEntries[i]);
		if (errno != 0)
		{
			return HTTPStatusCodes::clientError("Error parsing method " + std::to_string(i) + ".");
		}
		methods.push_back(method);
	}

	// Only upload if project and all methods are valid to prevent partial uploads.
	database -> addProject(project);

	std::queue<MethodIn> methodQueue;
	std::mutex queueLock;
	std::vector<std::thread> threads;

	for (MethodIn method : methods)
	{
		methodQueue.push(method);
	}
	for (int i = 0; i < MAX_THREADS; i++)
	{
		threads.push_back(std::thread(&DatabaseRequestHandler::singleUploadThread, this, ref(methodQueue), ref(queueLock), project));
	}
	for (int i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}
	if (errno == 0)
	{
		return HTTPStatusCodes::success("Your project has been successfully added to the database.");
	}
	else
	{
		return HTTPStatusCodes::clientError("An unexpected error occurred.");
	}
}

void DatabaseRequestHandler::singleUploadThread(std::queue<MethodIn> &methods, std::mutex &queueLock, ProjectIn project)
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

ProjectIn DatabaseRequestHandler::requestToProject(std::string request)
{
	errno = 0;
	// We retrieve the project information (projectData).
	std::string projectString = request.substr(0, request.find(ENTRY_DELIMITER_CHAR));
	std::vector<std::string> projectData = Utility::splitStringOn(projectString, FIELD_DELIMITER_CHAR);

	if (projectData.size() != PROJECT_DATA_SIZE)
	{
		errno = EILSEQ;
		return ProjectIn();
	}

	// We return the project information in the form of a Project.
	ProjectIn project;
	project.projectID  = Utility::safeStoll(projectData[0]);	// Converts string to long long.
	if (errno != 0)
	{
		return ProjectIn();
	}
	project.version    = Utility::safeStoll(projectData[1]);
	if (errno != 0)
	{
		return ProjectIn();
	}
	project.license    = projectData[2];
	project.name       = projectData[3];
	project.url        = projectData[4];
	project.owner.name = projectData[5];
	project.owner.mail = projectData[6];

	return project;
}

MethodIn DatabaseRequestHandler::dataEntryToMethod(std::string dataEntry)
{
	errno = 0;
	std::vector<std::string> methodData = Utility::splitStringOn(dataEntry, FIELD_DELIMITER_CHAR);

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

	std::vector<Author> authors;
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

std::string DatabaseRequestHandler::handleCheckRequest(std::string request)
{
	std::vector<Hash> hashes = Utility::splitStringOn(request, ENTRY_DELIMITER_CHAR);
	return handleCheckRequest(hashes);
}

std::string DatabaseRequestHandler::handleCheckRequest(std::vector<Hash> hashes)
{
	// Check if all requested hashes are invalid.
	for (int i = 0; i < hashes.size(); i++)
	{
		if (!isValidHash(hashes[i]))
		{
			return HTTPStatusCodes::clientError("Invalid hash presented.");
		}
	}

	// Request the specified hashes.
	std::vector<MethodOut> methods = getMethods(hashes);

	// Return retrieved data.
	std::string methodsStringFormat = methodsToString(methods, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR);
	if (!(methodsStringFormat == ""))
	{
		return HTTPStatusCodes::success(methodsStringFormat);
	}
	else
	{
		return HTTPStatusCodes::success("No results found.");
	}
}

std::vector<MethodOut> DatabaseRequestHandler::getMethods(std::vector<Hash> hashes)
{
	std::vector<std::future<std::vector<MethodOut>>> results;
	std::vector<std::thread> threads;
	std::queue<Hash> hashQueue;
	std::mutex queueLock;
	for (int i = 0; i < hashes.size(); i++)
	{
		hashQueue.push(hashes[i]);
	}
	for (int i = 0; i < MAX_THREADS; i++)
	{
		std::packaged_task<std::vector<MethodOut>()> task(
			bind(&DatabaseRequestHandler::singleHashToMethodsThread, this, ref(hashQueue), ref(queueLock)));
		results.push_back(task.get_future());
		threads.push_back(std::thread(move(task)));
	}
	for (int i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}
	std::vector<MethodOut> methods = {};
	for (int i = 0; i < results.size(); i++)
	{
		std::vector<MethodOut> newMethods = results[i].get();

		for (int j = 0; j < newMethods.size(); j++)
		{
			methods.push_back(newMethods[j]);
		}
	}
	return methods;
}

std::vector<ProjectOut> DatabaseRequestHandler::getProjects(std::queue<std::pair<ProjectID, Version>> keyQueue)
{
	std::vector<std::future<std::vector<ProjectOut>>> results;
	std::vector<std::thread> threads;
	std::mutex queueLock;
	for (int i = 0; i < MAX_THREADS; i++)
	{
		std::packaged_task<std::vector<ProjectOut>()> task(
			bind(&DatabaseRequestHandler::singleSearchProjectThread, this, ref(keyQueue), ref(queueLock)));
		results.push_back(task.get_future());
		threads.push_back(std::thread(move(task)));
	}
	for (int i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}
	std::vector<ProjectOut> projects = {};
	for (int i = 0; i < results.size(); i++)
	{
		std::vector<ProjectOut> newProjects = results[i].get();

		for (int j = 0; j < newProjects.size(); j++)
		{
			projects.push_back(newProjects[j]);
		}
	}
	return projects;
}

std::vector<MethodOut> DatabaseRequestHandler::singleHashToMethodsThread(std::queue<Hash> &hashes, std::mutex &queueLock)
{
	std::vector<MethodOut> methods;
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
		std::vector<MethodOut> newMethods = database->hashToMethods(hash);
		for (int j = 0; j < newMethods.size(); j++)
		{
			methods.push_back(newMethods[j]);
		}
	}
}

std::string DatabaseRequestHandler::methodsToString(std::vector<MethodOut> methods, char dataDelimiter, char methodDelimiter)
{
	std::vector<char> chars = {};
	while (!methods.empty())
	{
		MethodOut lastMethod     = methods.back();
		std::string hash              = lastMethod.hash;
		std::string projectID         = std::to_string(lastMethod.projectID);
		std::string version           = std::to_string(lastMethod.version);
		std::string name              = lastMethod.methodName;
		std::string fileLocation      = lastMethod.fileLocation;
		std::string lineNumber        = std::to_string(lastMethod.lineNumber);
		std::vector<std::string> authorIDs = lastMethod.authorIDs;
		std::string authorTotal       = std::to_string(authorIDs.size());

		// We initialize dataElements, which consists of the hash, projectID, version, name, fileLocation, lineNumber,
		// authorTotal and all the authorIDs.
		std::vector<std::string> dataElements = { hash, projectID, version, name, fileLocation, lineNumber, authorTotal };
		dataElements.insert(std::end(dataElements), std::begin(authorIDs), std::end(authorIDs));

		// Append 'chars' by the special dataElements separated by special characters.
		Utility::appendBy(chars, dataElements, dataDelimiter, methodDelimiter);
		methods.pop_back();
	}
	std::string result(chars.begin(), chars.end()); // Converts the vector of chars to a string.
	return result;
}

std::string DatabaseRequestHandler::handleExtractProjectsRequest(std::string request)
{
	errno = 0;
	std::vector<std::string> projectsData = Utility::splitStringOn(request, ENTRY_DELIMITER_CHAR);
	std::queue<std::pair<ProjectID, Version>> keyQueue;

	// We fill the queue with projectKeys, which identify a project uniquely.
	for (int i = 0; i < projectsData.size(); i++)
	{
		std::vector<std::string> projectData = Utility::splitStringOn(projectsData[i], FIELD_DELIMITER_CHAR);
		if (projectData.size() < 2)
		{
			errno = EILSEQ;
			return HTTPStatusCodes::clientError("The request failed. Each project should be provided a projectID and a version (in that order).");
		}
		ProjectID projectID = Utility::safeStoll(projectData[0]);
		Version version = Utility::safeStoll(projectData[1]);
		if (errno != 0)
		{
			return HTTPStatusCodes::clientError("The request failed. For each project, both the projectID and the version should be a long long int.");
		}

		std::pair<ProjectID, Version> key = std::make_pair(projectID, version);
		keyQueue.push(key);
	}

	std::vector<ProjectOut> projects = getProjects(keyQueue);
	return HTTPStatusCodes::success(projectsToString(projects, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR));
}

std::vector<ProjectOut> DatabaseRequestHandler::singleSearchProjectThread(std::queue<std::pair<ProjectID, Version>> &keys, std::mutex &queueLock)
{
	std::vector<ProjectOut> projects;
	while (true)
	{
		queueLock.lock();
		if (keys.size() <= 0)
		{
			queueLock.unlock();
			return projects;
		}
		std::pair<ProjectID, Version> key = keys.front();
		keys.pop();
		queueLock.unlock();

		ProjectID projectID = key.first;
		Version version = key.second;
		std::vector<ProjectOut> newProject = database->searchForProject(projectID, version);
		if (!newProject.empty())
		{
			projects.push_back(newProject[0]);
		}
	}
}

std::string DatabaseRequestHandler::projectsToString(std::vector<ProjectOut> projects, char dataDelimiter, char projectDelimiter)
{
	std::vector<char> chars = {};
	for (int i = 0; i < projects.size(); i++)
	{
		std::string projectID = std::to_string(projects[i].projectID);
		std::string version = std::to_string(projects[i].version);
		std::string license = projects[i].license;
		std::string name = projects[i].name;
		std::string url = projects[i].url;
		std::string ownerID = projects[i].ownerID;
		std::vector<Hash> hashes = projects[i].hashes;
		std::string hashesTotal = std::to_string(hashes.size());

		std::vector<std::string> dataElements = { projectID, version, license, name, url, ownerID };
		Utility::appendBy(chars, dataElements, dataDelimiter, projectDelimiter);
	}
	std::string result(chars.begin(), chars.end());

	if (result != "")
	{
		return result;
	}
	return "No results found.";
}

std::string DatabaseRequestHandler::handleGetAuthorIDRequest(std::string request)
{
	errno = 0;
	std::vector<std::string> authorStrings = Utility::splitStringOn(request, ENTRY_DELIMITER_CHAR);

	std::vector<Author> authors;

	for (int i = 0; i < authorStrings.size(); i++)
	{
		authors.push_back(datanEntryToAuthor(authorStrings[i]));
		if (errno != 0)
		{
			return HTTPStatusCodes::clientError("Error parsing author: " + authorStrings[i]);
		}
	}

	// Request the specified hashes.
	std::vector<std::tuple<Author, std::string>> authorIDs = getAuthorIDs(authors);

	if (authorIDs.size() <= 0)
	{
		return HTTPStatusCodes::success("No results found.");
	}

	return HTTPStatusCodes::success(authorsToString(authorIDs));
}

std::string DatabaseRequestHandler::authorsToString(std::vector<std::tuple<Author, std::string>> authors)
{
	std::vector<char> chars = {};
	while (!authors.empty())
	{
		std::tuple<Author, std::string> lastAuthorID = authors.back();
		std::string name = std::get<0>(lastAuthorID).name;
		std::string mail = std::get<0>(lastAuthorID).mail;
		std::string id = std::get<1>(lastAuthorID);

		// We initialize dataElements, which consists of the hash, projectID, version, name, fileLocation, lineNumber,
		// authorTotal and all the authorIDs.
		std::vector<std::string> dataElements = {name, mail, id};

		for (std::string data : dataElements)
		{
			Utility::appendBy(chars, data, FIELD_DELIMITER_CHAR);
		}

		// We should get rid of the last dataDelimiter if something is appended to 'chars',
		// which is precisely the case when it is non-empty.
		if (!chars.empty())
		{
			chars.pop_back();
		}

		// We end 'chars' with the methodDelimiter and indicate that we are done with 'lastMethod'.
		chars.push_back(ENTRY_DELIMITER_CHAR);
		authors.pop_back();
	}
	std::string result(chars.begin(), chars.end()); // Converts the vector of chars to a string.
	return result;
}

Author DatabaseRequestHandler::datanEntryToAuthor(std::string dataEntry)
{
	std::vector<std::string> authorData = Utility::splitStringOn(dataEntry, FIELD_DELIMITER_CHAR);

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

std::vector<std::tuple<Author, std::string>> DatabaseRequestHandler::getAuthorIDs(std::vector<Author> authors)
{
	std::vector<std::future<std::vector<std::tuple<Author, std::string>>>> results;
	std::vector<std::thread> threads;
	std::queue<Author> authorQueue;
	std::mutex queueLock;
	for (int i = 0; i < authors.size(); i++)
	{
		authorQueue.push(authors[i]);
	}
	for (int i = 0; i < MAX_THREADS; i++)
	{
		std::packaged_task<std::vector<std::tuple<Author, std::string>>()> task(
			bind(&DatabaseRequestHandler::singleAuthorToIDThread, this, ref(authorQueue), ref(queueLock)));
		results.push_back(task.get_future());
		threads.push_back(std::thread(move(task)));
	}
	for (int i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}
	std::vector<std::tuple<Author, std::string>> authorIDs;
	for (int i = 0; i < results.size(); i++)
	{
		std::vector<std::tuple<Author, std::string>> newAuthorIDs = results[i].get();

		for (int j = 0; j < newAuthorIDs.size(); j++)
		{
			authorIDs.push_back(newAuthorIDs[j]);
		}
	}
	return authorIDs;
}

std::vector<std::tuple<Author, std::string>> DatabaseRequestHandler::singleAuthorToIDThread(std::queue<Author> &authors, std::mutex &queueLock)
{
	std::vector<std::tuple<Author, std::string>> authorIDs;
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
		std::string newAuthorID = database->authorToId(author);
		if (newAuthorID != "")
		{
			authorIDs.push_back(make_tuple(author, newAuthorID));
		}
	}
}

std::string DatabaseRequestHandler::handleGetAuthorRequest(std::string request)
{
	std::vector<std::string> authorIds = Utility::splitStringOn(request, ENTRY_DELIMITER_CHAR);

	std::regex re(UUID_REGEX);

	for (int i = 0; i < authorIds.size(); i++)
	{
		if (!regex_match(authorIds[i], re))
		{
			return HTTPStatusCodes::clientError("Error parsing author id: " + authorIds[i]);
		}
	}

	// Request the specified hashes.
	std::vector<std::tuple<Author, std::string>> authors = getAuthors(authorIds);

	if (authors.size() <= 0)
	{
		return HTTPStatusCodes::success("No results found.");
	}

	return HTTPStatusCodes::success(authorsToString(authors));
}

std::vector<std::tuple<Author, std::string>> DatabaseRequestHandler::getAuthors(std::vector<std::string> authorIds)
{
	std::vector<std::future<std::vector<std::tuple<Author, std::string>>>> results;
	std::vector<std::thread> threads;
	std::queue<std::string> authorIdQueue;
	std::mutex queueLock;
	for (int i = 0; i < authorIds.size(); i++)
	{
		authorIdQueue.push(authorIds[i]);
	}
	for (int i = 0; i < MAX_THREADS; i++)
	{
		std::packaged_task<std::vector<std::tuple<Author, std::string>>()> task(
			bind(&DatabaseRequestHandler::singleIdToAuthorThread, this, ref(authorIdQueue), ref(queueLock)));
		results.push_back(task.get_future());
		threads.push_back(std::thread(move(task)));
	}
	for (int i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}
	std::vector<std::tuple<Author, std::string>> authors;
	for (int i = 0; i < results.size(); i++)
	{
		std::vector<std::tuple<Author, std::string>> newAuthors = results[i].get();

		for (int j = 0; j < newAuthors.size(); j++)
		{
			authors.push_back(newAuthors[j]);
		}
	}
	return authors;
}

std::vector<std::tuple<Author, std::string>> DatabaseRequestHandler::singleIdToAuthorThread(std::queue<std::string> &authorIds, std::mutex &queueLock)
{
	std::vector<std::tuple<Author, std::string>> authors;
	while (true)
	{
		queueLock.lock();
		if (authorIds.size() <= 0)
		{
			queueLock.unlock();
			return authors;
		}
		std::string id = authorIds.front();
		authorIds.pop();
		queueLock.unlock();
		Author newAuthor = database->idToAuthor(id);
		if (newAuthor.name != "" && newAuthor.mail != "")
		{
			authors.push_back(make_tuple(newAuthor, id));
		}
	}
}

std::string DatabaseRequestHandler::handleGetMethodsByAuthorRequest(std::string request)
{
	std::vector<std::string> authorIds = Utility::splitStringOn(request, ENTRY_DELIMITER_CHAR);

	std::regex re(UUID_REGEX);

	for (int i = 0; i < authorIds.size(); i++)
	{
		if (!regex_match(authorIds[i], re))
		{
			return HTTPStatusCodes::clientError("Error parsing author id: " + authorIds[i]);
		}
	}

	// Request the specified hashes.
	std::vector<std::tuple<MethodId, std::string>> methods = getMethodsByAuthor(authorIds);

	// Return retrieved data.
	std::string methodsStringFormat = methodIdsToString(methods);
	if (!(methodsStringFormat == ""))
	{
		return HTTPStatusCodes::success(methodsStringFormat);
	}
	else
	{
		return HTTPStatusCodes::success("No results found.");
	}
}

std::vector<std::tuple<MethodId, std::string>> DatabaseRequestHandler::getMethodsByAuthor(std::vector<std::string> authorIds)
{
	std::vector<std::future<std::vector<std::tuple<MethodId, std::string>>>> results;
	std::vector<std::thread> threads;
	std::queue<std::string> idQueue;
	std::mutex queueLock;
	for (int i = 0; i < authorIds.size(); i++)
	{
		idQueue.push(authorIds[i]);
	}
	for (int i = 0; i < MAX_THREADS; i++)
	{
		std::packaged_task<std::vector<std::tuple<MethodId, std::string>>()> task(
			bind(&DatabaseRequestHandler::singleAuthorToMethodsThread, this, ref(idQueue), ref(queueLock)));
		results.push_back(task.get_future());
		threads.push_back(std::thread(move(task)));
	}
	for (int i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}
	std::vector<std::tuple<MethodId, std::string>> methods = {};
	for (int i = 0; i < results.size(); i++)
	{
		std::vector<std::tuple<MethodId, std::string>> newMethods = results[i].get();

		for (int j = 0; j < newMethods.size(); j++)
		{
			methods.push_back(newMethods[j]);
		}
	}
	return methods;
}

std::vector<std::tuple<MethodId, std::string>> DatabaseRequestHandler::singleAuthorToMethodsThread(std::queue<std::string> &authorIds, std::mutex &queueLock)
{
	std::vector<std::tuple<MethodId, std::string>> methods;
	while (true)
	{
		queueLock.lock();
		if (authorIds.size() <= 0)
		{
			queueLock.unlock();
			return methods;
		}
		std::string authorId = authorIds.front();
		authorIds.pop();
		queueLock.unlock();
		std::vector<MethodId> newMethods = database->authorToMethods(authorId);
		for (int j = 0; j < newMethods.size(); j++)
		{
			methods.push_back(make_tuple(newMethods[j], authorId));
		}
	}
}

std::string DatabaseRequestHandler::methodIdsToString(std::vector<std::tuple<MethodId, std::string>> methods)
{
	std::vector<char> chars = {};
	while (!methods.empty())
	{
		std::tuple<MethodId, std::string> lastMethod = methods.back();
		std::string hash = std::get<0>(lastMethod).hash;
		std::string projectID = std::to_string(std::get<0>(lastMethod).projectId);
		std::string version = std::to_string(std::get<0>(lastMethod).version);
		std::string authorId = std::get<1>(lastMethod);

		// We initialize dataElements, which consists of the hash, projectID, version, name, fileLocation, lineNumber,
		// authorTotal and all the authorIDs.
		std::vector<std::string> dataElements = {authorId, hash, projectID, version};

		for (std::string data : dataElements)
		{
			Utility::appendBy(chars, data, FIELD_DELIMITER_CHAR);
		}

		// We should get rid of the last dataDelimiter if something is appended to 'chars',
		// which is precisely the case when it is non-empty.
		if (!chars.empty())
		{
			chars.pop_back();
		}

		// We end 'chars' with the methodDelimiter and indicate that we are done with 'lastMethod'.
		chars.push_back(ENTRY_DELIMITER_CHAR);
		methods.pop_back();
	}
	std::string result(chars.begin(), chars.end()); // Converts the vector of chars to a string.
	return result;
}
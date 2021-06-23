/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <ctime>
#include <regex>
#include <thread>
#include <future>
#include <utility>
#include <functional>
#include <math.h>
#include <unistd.h>

#include "DatabaseRequestHandler.h"
#include "HTTPStatus.h"
#include "Utility.h"

DatabaseRequestHandler::DatabaseRequestHandler(DatabaseHandler *database, std::string ip, int port)
{
	this->database = database;
	connectWithRetry(ip, port);
	if (errno != 0)
	{
		throw "Unable to connect to the database.";
	}
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
	for (int i = 3; i < data.size(); i++)
	{
		// Data before first delimiter.
		Hash hash = data[i].substr(0, data[i].find(FIELD_DELIMITER_CHAR));
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
	std::vector<std::string> dataEntries = Utility::splitStringOn(request, ENTRY_DELIMITER_CHAR);
	// Check if project is valid.
	ProjectIn project = requestToProject(dataEntries[0]);
	if (errno != 0)
	{
		// Project could not be parsed.
		return HTTPStatusCodes::clientError("Error parsing project data.");
	}

	if (errno != 0)
	{
		// Parser version could not be parsed.
		return HTTPStatusCodes::clientError("Error parsing parser version.");
	}
	std::vector<std::string> unchangedFiles;
	ProjectOut prevProject;
	bool newProject = true;
	if (dataEntries[1] != "")
	{
		long long prevVersion = Utility::safeStoll(dataEntries[1]);
		if (errno != 0)
		{
			// Previous version could not be parsed.
			return HTTPStatusCodes::clientError("Error parsing previous version.");
		}
		
		newProject = false;
		unchangedFiles = Utility::splitStringOn(dataEntries[2], FIELD_DELIMITER_CHAR);
		prevProject = database->searchForProject(project.projectID, prevVersion);
		if (errno != 0)
		{
			if (errno == ERANGE)
			{
				return HTTPStatusCodes::serverError("The database does not contain the provided version of the project.");
			}
			return HTTPStatusCodes::serverError("An error occurred while trying to locate the previous version of the project.");
		}
	}

	std::queue<MethodIn> methodQueue;
	for (int i = 3; i < dataEntries.size(); i++)
	{
		MethodIn method = dataEntryToMethod(dataEntries[i]);
		if (errno != 0)
		{
			return HTTPStatusCodes::clientError("Error parsing method " + std::to_string(i-2) + ".");
		}
		
		methodQueue.push(method);
		project.hashes.push_back(method.hash);
	}

	// Only upload if project and all methods are valid to prevent partial uploads.
	if (!tryUploadProjectWithRetry(project))
	{
		return HTTPStatusCodes::serverError("Failed to add project to database.");
	}

	handleUploadThreads(project, methodQueue, newProject, prevProject, unchangedFiles);
	if (errno != 0)
	{
		return HTTPStatusCodes::serverError("Unable to upload methods to the database.");
	}
	else
	{
		return HTTPStatusCodes::success("Your project has been successfully added to the database.");
	}
}

void DatabaseRequestHandler::handleUploadThreads(ProjectIn project, std::queue<MethodIn> methodQueue, bool newProject,
												 ProjectOut prevProject, std::vector<std::string> unchangedFiles)
{
	std::mutex queueLock;
	std::vector<std::thread> threads;
	for (int i = 0; i < MAX_THREADS; i++)
	{
		if (newProject)
		{
			threads.push_back(std::thread(&DatabaseRequestHandler::singleUploadThread, this, ref(methodQueue),
										  ref(queueLock), project, -1, project.parserVersion, newProject));
		}
		else
		{
			threads.push_back(std::thread(&DatabaseRequestHandler::singleUploadThread, this, ref(methodQueue),
										  ref(queueLock), project, prevProject.version, project.parserVersion,
										  newProject));
		}
		if (errno != 0)
		{
			return;
		}
	}
	for (int i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}

	if (!newProject)
	{
		threads.clear();
		handleUpdateUnchangedFilesThreads(project, prevProject, unchangedFiles);
	}
}

void DatabaseRequestHandler::handleUpdateUnchangedFilesThreads(ProjectIn project, ProjectOut prevProject,
															   std::vector<std::string> unchangedFiles)
{
	std::queue<std::pair<std::vector<Hash>, std::vector<std::string>>> hashFileQueue;
	std::vector<std::vector<Hash>> hashesList = toChunks(prevProject.hashes, HASHES_MAX_SIZE);
	std::vector<std::vector<std::string>> filesList = toChunks(unchangedFiles, FILES_MAX_SIZE);
	hashFileQueue = cartesianProductQueue(hashesList, filesList);

	std::mutex queueLock;
	std::vector<std::thread> threads;

	std::vector<std::future<std::vector<Hash>>> results;
	for (int i = 0; i < MAX_THREADS; i++)
	{
		std::packaged_task<std::vector<Hash>()> task(bind(&DatabaseRequestHandler::singleUpdateUnchangedFilesThread,
														  this, ref(hashFileQueue), ref(queueLock), project,
														  prevProject.version));
		if (errno != 0)
		{
			return;
		}
		results.push_back(task.get_future());
		threads.push_back(std::thread(move(task)));
	}
	for (int i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}
	threads.clear();

	std::vector<Hash> unchangedHashes = {};
	for (int i = 0; i < results.size(); i++)
	{
		std::vector<Hash> newHashes = results[i].get();

		for (int j = 0; j < newHashes.size(); j++)
		{
			unchangedHashes.push_back(newHashes[j]);
		}
	}

	project.hashes = unchangedHashes;
	database->addHashToProject(project, 0);
}

template <class T> std::vector<std::vector<T>> DatabaseRequestHandler::toChunks(std::vector<T> list, int chunkSize)
{
	std::vector<T> currentChunk = {};
	std::vector<std::vector<T>> chunks = {};
	for (T element : list)
	{
		currentChunk.push_back(element);
		if (currentChunk.size() >= chunkSize)
		{
			chunks.push_back(currentChunk);
			currentChunk.clear();
		}
	}
	if (currentChunk.size() > 0)
	{
		chunks.push_back(currentChunk);
	}

	return chunks;
}

template <class T1, class T2> std::queue<std::pair<std::vector<T1>, std::vector<T2>>>
DatabaseRequestHandler::cartesianProductQueue(std::vector<std::vector<T1>> listT1, std::vector<std::vector<T2>> listT2)
{
	std::queue<std::pair<std::vector<T1>, std::vector<T2>>> pairQueue;
	for (std::vector<T1> elemT1 : listT1)
	{
		for (std::vector<T2> elemT2 : listT2)
		{
			pairQueue.push(std::make_pair(elemT1, elemT2));
		}
	}

	return pairQueue;
}


void DatabaseRequestHandler::singleUploadThread(std::queue<MethodIn> &methods, std::mutex &queueLock, ProjectIn project, 
												long long prevVersion, long long parserVersion, bool newProject)
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
		addMethodWithRetry(method, project, prevVersion, parserVersion, newProject);
		if (errno != 0)
		{
			errno = ENETUNREACH;
		}
	}
}

std::vector<Hash> DatabaseRequestHandler::singleUpdateUnchangedFilesThread(
	std::queue<std::pair<std::vector<Hash>, std::vector<std::string>>> &hashFiles, std::mutex &queueLock,
	ProjectIn project, long long prevVersion)
{
	std::vector<Hash> hashes;
	while (true)
	{
		queueLock.lock();
		if (hashFiles.size() <= 0)
		{
			queueLock.unlock();
			return hashes;
		}
		std::pair<std::vector<Hash>, std::vector<std::string>> hashFile = hashFiles.front();
		hashFiles.pop();
		queueLock.unlock();
		std::vector<Hash> unchangedHashes = updateUnchangedFilesWithRetry(hashFile, project, prevVersion);
		if (errno != 0)
		{
			errno = ENETUNREACH;
			return {};
		}
		for (int j = 0; j < unchangedHashes.size(); j++)
		{
			hashes.push_back(unchangedHashes[j]);
		}
	}
}

ProjectIn DatabaseRequestHandler::requestToProject(std::string request)
{
	errno = 0;
	// We retrieve the project information (projectData).
	std::vector<std::string> projectData = Utility::splitStringOn(request, FIELD_DELIMITER_CHAR);

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
	project.versionHash		= projectData[2];
	project.license			= projectData[3];
	project.name			= projectData[4];
	project.url				= projectData[5];
	project.owner = Author(projectData[6], projectData[7]);
	project.parserVersion	= Utility::safeStoll(projectData[8]);
	if (errno != 0)
	{
		return ProjectIn();	
	}

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
		Author author(methodData[5 + 2 * i],methodData[6 + 2 * i]);
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
	if (errno != 0)
	{
		return HTTPStatusCodes::serverError("Unable to get methods from database.");
	}

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
		if (errno != 0)
		{
			errno = ENETUNREACH;
			return {};
		}
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
		if (errno != 0)
		{
			errno = ENETUNREACH;
			return {};
		}
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

std::vector<ProjectOut> DatabaseRequestHandler::getPrevProjects(std::queue<ProjectID> projectQueue)
{
	std::vector<std::future<std::vector<ProjectOut>>> results;
	std::vector<std::thread> threads;
	std::mutex queueLock;
	for (int i = 0; i < MAX_THREADS; i++)
	{
		std::packaged_task<std::vector<ProjectOut>()> task(
			bind(&DatabaseRequestHandler::singlePrevProjectThread, this, ref(projectQueue), ref(queueLock)));
		if (errno != 0)
		{
			errno = ENETUNREACH;
			return {};
		}
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
		std::vector<MethodOut> newMethods = hashToMethodsWithRetry(hash);
		if (errno != 0)
		{
			errno = ENETUNREACH;
			return {};
		}
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
		MethodOut lastMethod = methods.back();
		Hash hash						= lastMethod.hash;
		std::string projectID			= std::to_string(lastMethod.projectID);
		std::string startVersion		= std::to_string(lastMethod.startVersion);
		Hash startVersionHash			= lastMethod.startVersionHash;
		std::string endVersion			= std::to_string(lastMethod.endVersion);
		Hash endVersionHash				= lastMethod.endVersionHash;
		std::string name				= lastMethod.methodName;
		File fileLocation				= lastMethod.fileLocation;
		std::string lineNumber			= std::to_string(lastMethod.lineNumber);
		std::vector<AuthorID> authorIDs	= lastMethod.authorIDs;
		std::string authorTotal			= std::to_string(authorIDs.size());
		std::string parserVersion		= std::to_string(lastMethod.parserVersion);

		// We initialize dataElements, which consists of the hash, projectID, version, name, fileLocation, lineNumber,
		// authorTotal and all the authorIDs.
		std::vector<std::string> dataElements = {hash,		 projectID,		 startVersion, startVersionHash,
												 endVersion, endVersionHash, name,		   fileLocation,
												 lineNumber, parserVersion,  authorTotal};
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
	if (errno != 0)
	{
		return HTTPStatusCodes::serverError("Unable to get project(s) from the database.");
	}
	if (projects.size() <= 0)
	{
		return HTTPStatusCodes::success("No results found.");
	}
	return HTTPStatusCodes::success(projectsToString(projects, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR));
}

std::string DatabaseRequestHandler::handlePrevProjectsRequest(std::string request)
{
	errno = 0;
	std::vector<std::string> projectsData = Utility::splitStringOn(request, ENTRY_DELIMITER_CHAR);
	std::queue<ProjectID> projectQueue;

	// We fill the queue with projectKeys, which identify a project uniquely.
	for (int i = 0; i < projectsData.size(); i++)
	{
		ProjectID projectID = Utility::safeStoll(projectsData[i]);
		if (errno != 0)
		{
			return HTTPStatusCodes::clientError("The request failed. For each project, the projectID should be a long long int.");
		}

		projectQueue.push(projectID);
	}
	std::vector<ProjectOut> projects = getPrevProjects(projectQueue);

	if (errno == ENETUNREACH)
	{
		return HTTPStatusCodes::serverError("Unable to get project(s) from the database.");
	}
	if (projects.size() <= 0)
	{
		return HTTPStatusCodes::success("No results found.");
	}
	
	return HTTPStatusCodes::success(projectsToString(projects, FIELD_DELIMITER_CHAR, ENTRY_DELIMITER_CHAR));
}


std::vector<ProjectOut> DatabaseRequestHandler::singlePrevProjectThread(std::queue<ProjectID> &projectIDs, std::mutex &queueLock)
{
	std::vector<ProjectOut> projects;
	while (true)
	{
		queueLock.lock();
		if (projectIDs.size() <= 0)
		{
			queueLock.unlock();
			return projects;
		}
		ProjectID projectID = projectIDs.front();
		projectIDs.pop();
		queueLock.unlock();

		ProjectOut newProject = getPrevProjectWithRetry(projectID);
		if (newProject.projectID != -1)
		{
			projects.push_back(newProject);
		}
		return projects;
	}
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
		ProjectOut newProject = searchForProjectWithRetry(projectID, version);
		if (errno != 0 && errno != ERANGE)
		{
			return projects;
		}
		else if (errno != ERANGE)
		{
			projects.push_back(newProject);
		}
	}
}

std::string DatabaseRequestHandler::projectsToString(std::vector<ProjectOut> projects, char dataDelimiter,
													 char projectDelimiter)
{
	std::vector<char> chars = {};
	for (int i = 0; i < projects.size(); i++)
	{
		std::string projectID = std::to_string(projects[i].projectID);
		std::string version = std::to_string(projects[i].version);
		std::string versionHash = projects[i].versionHash;
		std::string license = projects[i].license;
		std::string name = projects[i].name;
		std::string url = projects[i].url;
		AuthorID ownerID = projects[i].ownerID;
		std::vector<Hash> hashes = projects[i].hashes;
		std::string hashesTotal = std::to_string(hashes.size());
		std::string parserVersion = std::to_string(projects[i].parserVersion);

		std::vector<std::string> dataElements = {projectID, version, versionHash, license, name, url, ownerID, parserVersion};
		Utility::appendBy(chars, dataElements, dataDelimiter, projectDelimiter);
	}
	std::string result(chars.begin(), chars.end());

	if (result == "")
	{
		return "No results found.";
	}
	else
	{
		return result;
	}
}

std::string DatabaseRequestHandler::authorsToString(std::vector<std::pair<Author, AuthorID>> authors)
{
	std::vector<char> chars = {};
	while (!authors.empty())
	{
		std::pair<Author, AuthorID> lastAuthorID = authors.back();
		std::string name = lastAuthorID.first.name;
		std::string mail = lastAuthorID.first.mail;
		AuthorID id = lastAuthorID.second;

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

	if (authorData.size() != 2)
	{
		errno = EILSEQ;
		Author author;
		return author;
	}

	Author author(authorData[0], authorData[1]);

	return author;
}

std::string DatabaseRequestHandler::handleGetAuthorRequest(std::string request)
{
	std::vector<AuthorID> authorIDs = Utility::splitStringOn(request, ENTRY_DELIMITER_CHAR);

	std::regex re(UUID_REGEX);

	for (int i = 0; i < authorIDs.size(); i++)
	{
		if (!regex_match(authorIDs[i], re))
		{
			return HTTPStatusCodes::clientError("Error parsing author id: " + authorIDs[i]);
		}
	}

	// Request the specified hashes.
	std::vector<std::pair<Author, AuthorID>> authors = getAuthors(authorIDs);
	if (errno != 0)
	{
		return HTTPStatusCodes::serverError("Unable to get authors from database.");
	}

	if (authors.size() <= 0)
	{
		return HTTPStatusCodes::success("No results found.");
	}

	return HTTPStatusCodes::success(authorsToString(authors));
}

std::vector<std::pair<Author, AuthorID>> DatabaseRequestHandler::getAuthors(std::vector<AuthorID> authorIDs)
{
	std::vector<std::future<std::vector<std::pair<Author, AuthorID>>>> results;
	std::vector<std::thread> threads;
	std::queue<AuthorID> authorIDQueue;
	std::mutex queueLock;
	for (int i = 0; i < authorIDs.size(); i++)
	{
		authorIDQueue.push(authorIDs[i]);
	}
	for (int i = 0; i < MAX_THREADS; i++)
	{
		std::packaged_task<std::vector<std::pair<Author, AuthorID>>()> task(
			bind(&DatabaseRequestHandler::singleIDToAuthorThread, this, ref(authorIDQueue), ref(queueLock)));
		if (errno != 0)
		{
			errno = ENETUNREACH;
			return {};
		}
		results.push_back(task.get_future());
		threads.push_back(std::thread(move(task)));
	}
	for (int i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}
	std::vector<std::pair<Author, AuthorID>> authors;
	for (int i = 0; i < results.size(); i++)
	{
		std::vector<std::pair<Author, AuthorID>> newAuthors = results[i].get();

		for (int j = 0; j < newAuthors.size(); j++)
		{
			authors.push_back(newAuthors[j]);
		}
	}
	return authors;
}

std::vector<std::pair<Author, AuthorID>> DatabaseRequestHandler::singleIDToAuthorThread(std::queue<AuthorID> &authorIDs, std::mutex &queueLock)
{
	std::vector<std::pair<Author, AuthorID>> authors;
	while (true)
	{
		queueLock.lock();
		if (authorIDs.size() <= 0)
		{
			queueLock.unlock();
			return authors;
		}
		AuthorID id = authorIDs.front();
		authorIDs.pop();
		queueLock.unlock();
		Author newAuthor = idToAuthorWithRetry(id);
		if (errno != 0)
		{
			errno = ENETUNREACH;
			return {};
		}
		if (newAuthor.name != "" && newAuthor.mail != "")
		{
			authors.push_back(make_pair(newAuthor, id));
		}
	}
}

std::string DatabaseRequestHandler::handleGetMethodsByAuthorRequest(std::string request)
{
	std::vector<AuthorID> authorIDs = Utility::splitStringOn(request, ENTRY_DELIMITER_CHAR);

	std::regex re(UUID_REGEX);

	for (int i = 0; i < authorIDs.size(); i++)
	{
		if (!regex_match(authorIDs[i], re))
		{
			return HTTPStatusCodes::clientError("Error parsing author id: " + authorIDs[i]);
		}
	}

	// Request the specified hashes.
	std::vector<std::pair<MethodID, AuthorID>> methods = getMethodsByAuthor(authorIDs);
	if (errno != 0)
	{
		return HTTPStatusCodes::serverError("Unable to get methods from database.");
	}

	// Return retrieved data.
	std::string methodsStringFormat = methodIDsToString(methods);
	if (!(methodsStringFormat == ""))
	{
		return HTTPStatusCodes::success(methodsStringFormat);
	}
	else
	{
		return HTTPStatusCodes::success("No results found.");
	}
}

std::vector<std::pair<MethodID, AuthorID>> DatabaseRequestHandler::getMethodsByAuthor(std::vector<AuthorID> authorIDs)
{
	std::vector<std::future<std::vector<std::pair<MethodID, AuthorID>>>> results;
	std::vector<std::thread> threads;
	std::queue<AuthorID> idQueue;
	std::mutex queueLock;
	for (int i = 0; i < authorIDs.size(); i++)
	{
		idQueue.push(authorIDs[i]);
	}
	for (int i = 0; i < MAX_THREADS; i++)
	{
		std::packaged_task<std::vector<std::pair<MethodID, AuthorID>>()> task(
			bind(&DatabaseRequestHandler::singleAuthorToMethodsThread, this, ref(idQueue), ref(queueLock)));
		if (errno != 0)
		{
			errno = ENETUNREACH;
			return {};
		}
		results.push_back(task.get_future());
		threads.push_back(std::thread(move(task)));
	}
	for (int i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}
	std::vector<std::pair<MethodID, AuthorID>> methods = {};
	for (int i = 0; i < results.size(); i++)
	{
		std::vector<std::pair<MethodID, AuthorID>> newMethods = results[i].get();

		for (int j = 0; j < newMethods.size(); j++)
		{
			methods.push_back(newMethods[j]);
		}
	}
	return methods;
}

std::vector<std::pair<MethodID, AuthorID>> DatabaseRequestHandler::singleAuthorToMethodsThread(std::queue<AuthorID> &authorIDs, std::mutex &queueLock)
{
	std::vector<std::pair<MethodID, AuthorID>> methods;
	while (true)
	{
		queueLock.lock();
		if (authorIDs.size() <= 0)
		{
			queueLock.unlock();
			return methods;
		}
		AuthorID authorID = authorIDs.front();
		authorIDs.pop();
		queueLock.unlock();
		std::vector<MethodID> newMethods = authorToMethodsWithRetry(authorID);
		if (errno != 0)
		{
			errno = ENETUNREACH;
			return {};
		}
		for (int j = 0; j < newMethods.size(); j++)
		{
			methods.push_back(make_pair(newMethods[j], authorID));
		}
	}
}

std::string DatabaseRequestHandler::methodIDsToString(std::vector<std::pair<MethodID, AuthorID>> methods)
{
	std::vector<char> chars = {};
	while (!methods.empty())
	{
		std::pair<MethodID, AuthorID> lastMethod = methods.back();
		Hash hash = lastMethod.first.hash;
		std::string projectID = std::to_string(lastMethod.first.projectID);
		std::string startVersion = std::to_string(lastMethod.first.startVersion);
		AuthorID authorID = lastMethod.second;

		// We initialize dataElements, which consists of the hash, projectID, version, name, fileLocation, lineNumber,
		// authorTotal and all the authorIDs.
		std::vector<std::string> dataElements = {authorID, hash, projectID, startVersion};

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

template <class T> T DatabaseRequestHandler::queryWithRetry(std::function<T()> function)
{
	errno = 0;
	int retries = 0;
	T items;
	do
	{
		items = function();
		if (errno != 0 && errno != ERANGE)
		{
			usleep(pow(2,retries) * RETRY_SLEEP);
			retries++;
		}
	} 
	while (errno != 0 && errno != ERANGE && retries <= MAX_RETRIES);
	if (retries > MAX_RETRIES)
	{
		errno = ENETUNREACH;
		//return NULL;
	}
	return items;
}

std::tuple<> DatabaseRequestHandler::connectWithRetry(std::string ip, int port)
{
	std::function<std::tuple<>()> function = [ip, port, this]()
	{
		this->database->connect(ip, port);
		return std::make_tuple();
	};
	return queryWithRetry<std::tuple<>>(function);
}

bool DatabaseRequestHandler::tryUploadProjectWithRetry(ProjectIn project)
{
	std::function<bool()> function = [project, this]()
	{ 
		return this->database->addProject(project); 
	};
	return queryWithRetry<bool>(function);
}

std::tuple<> DatabaseRequestHandler::addMethodWithRetry(MethodIn method, ProjectIn project, long long prevVersion,
												long long parserVersion, bool newProject)
{
	std::function<std::tuple<>()> function = [method, project, prevVersion, parserVersion, newProject, this]()
	{
		this->database->addMethod(method, project, prevVersion, parserVersion, newProject);
		return std::make_tuple();
	};
	return queryWithRetry<std::tuple<>>(function);
}

std::vector<Hash>
DatabaseRequestHandler::updateUnchangedFilesWithRetry(std::pair<std::vector<Hash>, std::vector<File>> hashFile,
													  ProjectIn project, long long prevVersion)
{
	std::vector<Hash> hash = hashFile.first;
	std::vector<std::string> file = hashFile.second;
	std::function<std::vector<Hash>()> function = [hash, file, project, prevVersion, this]()
	{
		return this->database->updateUnchangedFiles(hash, file, project, prevVersion);
	};
	return queryWithRetry<std::vector<Hash>>(function);
}

std::vector<MethodOut> DatabaseRequestHandler::hashToMethodsWithRetry(Hash hash)
{
	std::function<std::vector<MethodOut>()> function = [hash, this]()
	{ 
		return this->database->hashToMethods(hash); 
	};
	return queryWithRetry<std::vector<MethodOut>>(function);
}

ProjectOut DatabaseRequestHandler::searchForProjectWithRetry(ProjectID projectID, Version version)
{
	std::function<ProjectOut()> function = [projectID, version, this]()
	{ 
		return this->database->searchForProject(projectID, version); 
	};
	return queryWithRetry<ProjectOut>(function);
}

ProjectOut DatabaseRequestHandler::getPrevProjectWithRetry(ProjectID projectID)
{
	std::function<ProjectOut()> function = [projectID, this]()
	{ 
		return this->database->prevProject(projectID); 
	};
	return queryWithRetry<ProjectOut>(function);
}

Author DatabaseRequestHandler::idToAuthorWithRetry(AuthorID id)
{
	std::function<Author()> function = [id, this]() 
	{
		Author author;
		author = this->database->idToAuthor(id);
		return author;
	};
	return queryWithRetry<Author>(function);
}

std::vector<MethodID> DatabaseRequestHandler::authorToMethodsWithRetry(AuthorID authorID)
{
	std::function<std::vector<MethodId>()> function = [authorId, this]()
	{ 
		return this->database->authorToMethods(authorId); 
	};
	return queryWithRetry<std::vector<MethodId>>(function);
}

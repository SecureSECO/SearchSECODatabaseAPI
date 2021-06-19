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

#include "DatabaseRequestHandler.h"
#include "HTTPStatus.h"
#include "Utility.h"

DatabaseRequestHandler::DatabaseRequestHandler(DatabaseHandler *database, std::string ip, int port)
{
	this->database = database;
	connectWithRetry(ip, port);
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

	std::queue<std::pair<std::vector<Hash>, std::vector<std::string>>> hashFileQueue;
	if (!newProject)
	{
		std::vector<std::vector<Hash>> hashesList = toChunks(prevProject.hashes, HASHES_MAX_SIZE);
		std::vector<std::vector<std::string>> filesList = toChunks(unchangedFiles, FILES_MAX_SIZE);
		hashFileQueue = cartesianProductQueue(hashesList, filesList);
	}

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
										  ref(queueLock), project, prevProject.version, project.parserVersion, newProject));
		}
		if (errno != 0)
		{
			return HTTPStatusCodes::serverError("Unable to upload methods to the database.");
		}
	}
	for (int i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}

	if (!newProject)
	{
		threads.clear();
		std::vector<std::future<std::vector<Hash>>> results;
		for (int i = 0; i < MAX_THREADS; i++)
		{
			std::packaged_task<std::vector<Hash>()> task(bind(&DatabaseRequestHandler::singleUpdateUnchangedFilesThread,
															  this, ref(hashFileQueue), ref(queueLock), project,
															  prevProject.version));
			if (errno != 0)
			{
				return HTTPStatusCodes::serverError("Unable to upload methods to the database.");
			}
			results.push_back(task.get_future());
			threads.push_back(std::thread(move(task)));
		}
		for (int i = 0; i < threads.size(); i++)
		{
			threads[i].join();
		}
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
	if (errno == 0)
	{
		return HTTPStatusCodes::success("Your project has been successfully added to the database.");
	}
	else
	{
		return HTTPStatusCodes::clientError("An unexpected error occurred.");
	}
}

std::vector<std::vector<std::string>> DatabaseRequestHandler::toChunks(std::vector<std::string> list, int chunkSize)
{
	std::vector<std::string> currentChunk = {};
	std::vector<std::vector<std::string>> chunks = {};
	for (std::string element : list)
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

std::queue<std::pair<std::vector<std::string>, std::vector<std::string>>>
DatabaseRequestHandler::cartesianProductQueue(std::vector<std::vector<std::string>> firstList,
											  std::vector<std::vector<std::string>> secondList)
{
	std::queue<std::pair<std::vector<std::string>, std::vector<std::string>>> pairQueue;
	for (std::vector<std::string> firstElem : firstList)
	{
		for (std::vector<std::string> secondElem : secondList)
		{
			pairQueue.push(std::make_pair(firstElem, secondElem));
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
		std::string hash					= lastMethod.hash;
		std::string projectID				= std::to_string(lastMethod.projectID);
		std::string startVersion			= std::to_string(lastMethod.startVersion);
		std::string startVersionHash		= lastMethod.startVersionHash;
		std::string endVersion				= std::to_string(lastMethod.endVersion);
		std::string endVersionHash			= lastMethod.endVersionHash;
		std::string name					= lastMethod.methodName;
		std::string fileLocation			= lastMethod.fileLocation;
		std::string lineNumber				= std::to_string(lastMethod.lineNumber);
		std::vector<std::string> authorIDs	= lastMethod.authorIDs;
		std::string authorTotal				= std::to_string(authorIDs.size());
		std::string parserVersion			= std::to_string(lastMethod.parserVersion);

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

	if (errno != 0 && errno != ERANGE)
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

		ProjectOut newProject = database->prevProject(projectID);
		if (errno != 0 && errno != ERANGE)
		{
			errno = ENETUNREACH;
			return projects;
		}
		if (errno != ERANGE)
		{
			projects.push_back(newProject);
		}
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
		if (errno != ERANGE)
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
		std::string ownerID = projects[i].ownerID;
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

	if (authorData.size() != 2)
	{
		errno = EILSEQ;
		Author author("","");
		return author;
	}

	Author author(authorData[0], authorData[1]);

	return author;
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
		Author newAuthor = idToAuthorWithRetry(id);
		if (errno != 0)
		{
			errno = ENETUNREACH;
			return {};
		}
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
	if (errno != 0)
	{
		return HTTPStatusCodes::serverError("Unable to get methods from database.");
	}

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
		std::vector<MethodId> newMethods = authorToMethodsWithRetry(authorId);
		if (errno != 0)
		{
			errno = ENETUNREACH;
			return {};
		}
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
		std::string startVersion = std::to_string(std::get<0>(lastMethod).startVersion);
		std::string authorId = std::get<1>(lastMethod);

		// We initialize dataElements, which consists of the hash, projectID, version, name, fileLocation, lineNumber,
		// authorTotal and all the authorIDs.
		std::vector<std::string> dataElements = {authorId, hash, projectID, startVersion};

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

void DatabaseRequestHandler::connectWithRetry(std::string ip, int port)
{
	int retries = 0;
	errno = 0;
	database->connect(ip, port);
	if (errno != 0)
	{
		while (retries < MAX_RETRIES)
		{
			database->connect(ip, port);
			if (errno == 0)
			{
				return;
			}
			retries++;
		}
		throw "Unable to connect to database.";
	}
	errno = 0;
}

bool DatabaseRequestHandler::tryUploadProjectWithRetry(ProjectIn project)
{
	int retries = 0;
	database->addProject(project);
	if (errno != 0)
	{
		while (retries < MAX_RETRIES)
		{
			database->addProject(project);
			if (errno == 0)
			{
				return true;
			}
			retries++;
		}
		return false;
	}
	return true;
}

void DatabaseRequestHandler::addMethodWithRetry(MethodIn method, ProjectIn project, long long prevVersion, long long parserVersion, bool newProject)
{
	int retries = 0;
	database->addMethod(method, project, prevVersion, parserVersion, newProject);

	if (errno != 0 && errno != ERANGE)
	{
		while (retries < MAX_RETRIES)
		{
			database->addMethod(method, project, prevVersion, parserVersion, newProject);
			if (errno == 0 && errno != ERANGE)
			{
				return;
			}
			retries++;
			errno = 0;
		}
		errno = ENETUNREACH;
		return;
	}
}

std::vector<Hash> DatabaseRequestHandler::updateUnchangedFilesWithRetry(std::pair<std::vector<Hash>, std::vector<std::string>> hashFile, ProjectIn project, long long prevVersion)
{
	int retries = 0;
	std::vector<Hash> hashes;
	hashes = database->updateUnchangedFiles(hashFile.first, hashFile.second, project, prevVersion);
	if (errno != 0)
	{
		while (retries < MAX_RETRIES)
		{
			hashes = database->updateUnchangedFiles(hashFile.first, hashFile.second, project, prevVersion);
			if (errno == 0)
			{
				return hashes;
			}
			retries++;
		}
		errno = ENETUNREACH;
		return {};
	}
	return hashes;
}


std::vector<MethodOut> DatabaseRequestHandler::hashToMethodsWithRetry(Hash hash)
{
	int retries = 0;
	std::vector<MethodOut> methods;
	methods = database->hashToMethods(hash);
	if (errno != 0)
	{
		while (retries < MAX_RETRIES)
		{
			methods = database->hashToMethods(hash);
			if (errno == 0)
			{
				return methods;
			}
			retries++;
		}
		errno = ENETUNREACH;
		return {};
	}
	return methods;
}

ProjectOut DatabaseRequestHandler::searchForProjectWithRetry(ProjectID projectID, Version version)
{
	errno = 0;
	int retries = 0;
	ProjectOut project;
	project = database->searchForProject(projectID, version);
	if (errno != 0 && errno != ERANGE)
	{
		while (retries < MAX_RETRIES)
		{
			project = database->searchForProject(projectID, version);
			if (errno == 0 || errno == ERANGE)
			{
				return project;
			}
			retries++;
		}
		errno = ENETUNREACH;
	}
	return project;
}

Author DatabaseRequestHandler::idToAuthorWithRetry(std::string id)
{
	int retries = 0;
	Author author("","");
	author = database->idToAuthor(id);
	if (errno != 0)
	{
		while (retries < MAX_RETRIES)
		{
			author = database->idToAuthor(id);
			if (errno == 0)
			{
				return author;
			}
			retries++;
		}
		errno = ENETUNREACH;
		return author;
	}
	return author;
}

std::vector<MethodId> DatabaseRequestHandler::authorToMethodsWithRetry(std::string authorId)
{
	int retries = 0;
	std::vector<MethodId> methods;
	methods = database->authorToMethods(authorId);
	if (errno != 0)
	{
		while (retries < MAX_RETRIES)
		{
			methods = database->authorToMethods(authorId);
			if (errno == 0)
			{
				return methods;
			}
			retries++;
		}
		errno = ENETUNREACH;
		return {};
	}
	return methods;
}

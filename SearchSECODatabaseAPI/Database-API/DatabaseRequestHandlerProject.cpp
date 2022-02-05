/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "Definitions.h"
#include "DatabaseRequestHandler.h"
#include "HTTPStatus.h"
#include "Utility.h"

#include <future>
#include <thread>
#include <regex>

std::string DatabaseRequestHandler::handleCheckUploadRequest(std::string request, std::string client)
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

	std::string uploadResult = handleUploadRequest(request, client);
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

std::string DatabaseRequestHandler::handleUploadRequest(std::string request, std::string client)
{
	std::vector<std::string> dataEntries = Utility::splitStringOn(request, ENTRY_DELIMITER_CHAR);
	// Check if project is valid.
	ProjectIn project = requestToProject(dataEntries[0]);
	if (errno != 0)
	{
		// Project could not be parsed.
		return HTTPStatusCodes::clientError("Error parsing project data.");
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
		if (errno == ERANGE)
		{
			return HTTPStatusCodes::serverError("The database does not contain the provided version of the project.");
		}
		else if (errno != 0)
		{
			return HTTPStatusCodes::serverError(
				"An error occurred while trying to locate the previous version of the project.");
		}
	}

	std::queue<MethodIn> methodQueue;

	std::map<std::string, int> extensionOccurences;

	for (int i = 3; i < dataEntries.size(); i++)
	{
		MethodIn method = dataEntryToMethod(dataEntries[i]);
		if (errno != 0)
		{
			return HTTPStatusCodes::clientError("Error parsing method " + std::to_string(i-2) + ".");
		}

		if (method.vulnCode != "")
		{
			stats->vulnCounter->Add({{"Node", stats->myIP}, {"Client", client}}).Increment();
		}
		
		++extensionOccurences[getExtension(method.fileLocation)];
		methodQueue.push(method);
		project.hashes.push_back(method.hash);
	}

	for (std::pair<std::string, int> extension : extensionOccurences)
	{
		stats->methodCounter->Add({{"Node", stats->myIP}, {"Client", client}, {"Extension", extension.first}}).Increment(extension.second);
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
	return HTTPStatusCodes::success("Your project has been successfully added to the database.");
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

std::vector<Hash>
DatabaseRequestHandler::updateUnchangedFilesWithRetry(std::pair<std::vector<Hash>, std::vector<File>> hashFile,
													  ProjectIn project, long long prevVersion)
{
	std::vector<Hash> hash = hashFile.first;
	std::vector<File> file = hashFile.second;
	std::function<std::vector<Hash>()> function = [hash, file, project, prevVersion, this]() {
		return this->database->updateUnchangedFiles(hash, file, project, prevVersion);
	};
	return Utility::queryWithRetry<std::vector<Hash>>(function);
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
	project.projectID  = Utility::safeStoll(projectData[0]);
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
			return HTTPStatusCodes::clientError(
				"The request failed. Each project should be provided a projectID and a version (in that order).");
		}
		ProjectID projectID = Utility::safeStoll(projectData[0]);
		Version version = Utility::safeStoll(projectData[1]);
		if (errno != 0)
		{
			return HTTPStatusCodes::clientError(
				"The request failed. For each project, both the projectID and the version should be a long long int.");
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

	// We fill the queue with projectIDs to retrieve the latest version of the projects.
	for (int i = 0; i < projectsData.size(); i++)
	{
		ProjectID projectID = Utility::safeStoll(projectsData[i]);
		if (errno != 0)
		{
			return HTTPStatusCodes::clientError(
				"The request failed. For each project, the projectID should be a long long int.");
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


std::vector<ProjectOut> DatabaseRequestHandler::singlePrevProjectThread(std::queue<ProjectID> &projectIDs,
																		std::mutex &queueLock)
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

std::vector<ProjectOut>
DatabaseRequestHandler::singleSearchProjectThread(std::queue<std::pair<ProjectID, Version>> &keys,
												  std::mutex &queueLock)
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

		std::vector<std::string> dataElements = {projectID, version, versionHash, license,
												 name,		url,	 ownerID,	  parserVersion};
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

bool DatabaseRequestHandler::tryUploadProjectWithRetry(ProjectIn project)
{
	std::function<bool()> function = [project, this]() { return this->database->addProject(project); };
	return Utility::queryWithRetry<bool>(function);
}

ProjectOut DatabaseRequestHandler::searchForProjectWithRetry(ProjectID projectID, Version version)
{
	std::function<ProjectOut()> function = 
		[projectID, version, this]() { return this->database->searchForProject(projectID, version); };
	return Utility::queryWithRetry<ProjectOut>(function);
}

ProjectOut DatabaseRequestHandler::getPrevProjectWithRetry(ProjectID projectID)
{
	std::function<ProjectOut()> function = [projectID, this]() { return this->database->prevProject(projectID); };
	return Utility::queryWithRetry<ProjectOut>(function);
}
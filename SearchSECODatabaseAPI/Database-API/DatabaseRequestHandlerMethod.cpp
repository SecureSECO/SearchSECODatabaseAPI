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
		Author author(methodData[5 + 2 * i], methodData[6 + 2 * i]);
		authors.push_back(author);
	}
	method.authors = authors;

	return method;
}

std::string DatabaseRequestHandler::getExtension(std::string file)
{
	size_t loc = file.find_last_of('.');
	return file.substr(loc, file.size());
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

std::vector<MethodOut> DatabaseRequestHandler::singleHashToMethodsThread(std::queue<Hash> &hashes,
																		 std::mutex &queueLock)
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

std::string DatabaseRequestHandler::methodsToString(std::vector<MethodOut> methods, char dataDelimiter,
													char methodDelimiter)
{
	std::vector<char> chars = {};
	while (!methods.empty())
	{
		MethodOut lastMethod = methods.back();
		Hash hash = lastMethod.hash;
		std::string projectID = std::to_string(lastMethod.projectID);
		std::string startVersion = std::to_string(lastMethod.startVersion);
		Hash startVersionHash = lastMethod.startVersionHash;
		std::string endVersion = std::to_string(lastMethod.endVersion);
		Hash endVersionHash = lastMethod.endVersionHash;
		std::string name = lastMethod.methodName;
		File fileLocation = lastMethod.fileLocation;
		std::string lineNumber = std::to_string(lastMethod.lineNumber);
		std::vector<AuthorID> authorIDs = lastMethod.authorIDs;
		std::string authorTotal = std::to_string(authorIDs.size());
		std::string parserVersion = std::to_string(lastMethod.parserVersion);

		// We initialize dataElements, which consists of the hash, projectID, version, name, fileLocation, lineNumber,
		// authorTotal and all the authorIDs.
		std::vector<std::string> dataElements = {hash,		 projectID,		 startVersion, startVersionHash,
												 endVersion, endVersionHash, name,		   fileLocation,
												 lineNumber, parserVersion,	 authorTotal};
		dataElements.insert(std::end(dataElements), std::begin(authorIDs), std::end(authorIDs));

		// Append 'chars' by the special dataElements separated by special characters.
		Utility::appendBy(chars, dataElements, dataDelimiter, methodDelimiter);
		methods.pop_back();
	}
	std::string result(chars.begin(), chars.end()); // Converts the vector of chars to a string.
	return result;
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

std::vector<std::pair<MethodID, AuthorID>>
DatabaseRequestHandler::singleAuthorToMethodsThread(std::queue<AuthorID> &authorIDs, std::mutex &queueLock)
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

		// We initialize dataElements, which consists of the authorID, hash, projectID and startVersion.
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

std::tuple<> DatabaseRequestHandler::addMethodWithRetry(MethodIn method, ProjectIn project, long long prevVersion,
														long long parserVersion, bool newProject)
{
	std::function<std::tuple<>()> function = [method, project, prevVersion, parserVersion, newProject, this]() {
		this->database->addMethod(method, project, prevVersion, parserVersion, newProject);
		return std::make_tuple();
	};
	return queryWithRetry<std::tuple<>>(function);
}

std::vector<MethodOut> DatabaseRequestHandler::hashToMethodsWithRetry(Hash hash)
{
	std::function<std::vector<MethodOut>()> function = [hash, this]() { return this->database->hashToMethods(hash); };
	return queryWithRetry<std::vector<MethodOut>>(function);
}

std::vector<MethodID> DatabaseRequestHandler::authorToMethodsWithRetry(AuthorID authorID)
{
	std::function<std::vector<MethodID>()> function = [authorID, this]() {
		return this->database->authorToMethods(authorID);
	};
	return queryWithRetry<std::vector<MethodID>>(function);
}

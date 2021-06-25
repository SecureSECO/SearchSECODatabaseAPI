/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "DatabaseRequestHandler.cpp"

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

std::vector<std::pair<Author, AuthorID>> DatabaseRequestHandler::singleIDToAuthorThread(std::queue<AuthorID> &authorIDs,
																						std::mutex &queueLock)
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

Author DatabaseRequestHandler::idToAuthorWithRetry(AuthorID id)
{
	std::function<Author()> function = [id, this]() {
		Author author;
		author = this->database->idToAuthor(id);
		return author;
	};
	return queryWithRetry<Author>(function);
}
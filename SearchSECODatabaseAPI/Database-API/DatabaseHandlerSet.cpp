/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "DatabaseHandler.h"
#include "Utility.h"

#include <iostream>
#include <string>
#include <unistd.h>

bool DatabaseHandler::addProject(ProjectIn project)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(insertProject);

	// Bind the variables in the statement.
	cass_statement_bind_int64_by_name(query, "projectID", project.projectID);
	cass_statement_bind_int64_by_name(query, "versiontime", project.version);
	cass_statement_bind_string_by_name(query, "versionhash", project.versionHash.c_str());
	cass_statement_bind_string_by_name(query, "license", project.license.c_str());
	cass_statement_bind_string_by_name(query, "name", project.name.c_str());
	cass_statement_bind_string_by_name(query, "url", project.url.c_str());
	cass_statement_bind_uuid_by_name(query, "ownerid", createAuthorIfNotExists(project.owner));
	cass_statement_bind_int64_by_name(query, "parserversion", project.parserVersion);

	int size = project.hashes.size();

	CassCollection *hashes = cass_collection_new(CASS_COLLECTION_TYPE_SET, size);

	// Add the hashes, but no more than HASHES_INSERT_MAX
	for (int i = 0; i < std::min(HASHES_INSERT_MAX, size); i++)
	{
		CassUuid hash;
		cass_uuid_from_string(Utility::hashToUUIDString(project.hashes[i]).c_str(), &hash);
		cass_collection_append_uuid(hashes, hash);
	}

	cass_statement_bind_collection_by_name(query, "hashes", hashes);

	CassFuture *queryFuture = cass_session_execute(connection, query);

	// Statement objects can be freed immediately after being executed.
	cass_statement_free(query);

	// Block until the query has finished and obtain the error code.
	CassError rc = cass_future_error_code(queryFuture);

	if (rc != 0)
	{
		const char *message;
		size_t messageLength;
		cass_future_error_message(queryFuture, &message, &messageLength);
		fprintf(stderr, "Unable to add project: '%.*s'\n", (int)messageLength, message);
		errno = ENETUNREACH;
		return false;
	}

	cass_future_free(queryFuture);

	// Add the extra hashes if necessary
	if (size > HASHES_INSERT_MAX)
	{
		addHashToProject(project, HASHES_INSERT_MAX);
	}
	return true;
}

void DatabaseHandler::addHashToProject(ProjectIn project, int index)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(addHashesToProject);

	// Bind the variables in the statement.
	cass_statement_bind_int64_by_name(query, "projectID", project.projectID);
	cass_statement_bind_int64_by_name(query, "versiontime", project.version);

	int size = project.hashes.size();
	CassCollection *hashes = cass_collection_new(CASS_COLLECTION_TYPE_SET, size);
	for (int i = index; i < std::min(index + HASHES_INSERT_MAX, size); i++)
	{
		CassUuid hash;
		cass_uuid_from_string(Utility::hashToUUIDString(project.hashes[i]).c_str(), &hash);
		cass_collection_append_uuid(hashes, hash);
	}
	cass_statement_bind_collection_by_name(query, "hashes", hashes);

	CassFuture *queryFuture = cass_session_execute(connection, query);

	// Statement objects can be freed immediately after being executed.
	cass_statement_free(query);

	// Block until the query has finished and obtain the error code.
	CassError rc = cass_future_error_code(queryFuture);

	if (rc != 0)
	{
		const char *message;
		size_t messageLength;
		cass_future_error_message(queryFuture, &message, &messageLength);
		fprintf(stderr, "Unable to add hash to project: '%.*s'\n", (int)messageLength, message);
		errno = ENETUNREACH;
	}

	cass_future_free(queryFuture);

	// Recursively add hashes as long as necessary
	if (size > index + HASHES_INSERT_MAX)
	{
		addHashToProject(project, index + HASHES_INSERT_MAX);
	}
}

void DatabaseHandler::addMethod(MethodIn method, ProjectIn project, long long prevVersion, long long parserVersion,
								bool newProject)
{
	errno = 0;

	bool newMethod = true;
	if (!newProject)
	{
		CassStatement *query = cass_prepared_bind(selectMethod);

		std::string hashUUID = Utility::hashToUUIDString(method.hash);

		// Bind the variables in the statement.
		CassUuid uuid;
		cass_uuid_from_string(hashUUID.c_str(), &uuid);
		cass_statement_bind_uuid_by_name(query, "method_hash", uuid);
		cass_statement_bind_int64_by_name(query, "projectID", project.projectID);
		cass_statement_bind_string_by_name(query, "file", method.fileLocation.c_str());

		CassFuture *queryFuture = cass_session_execute(connection, query);
		if (cass_future_error_code(queryFuture) == CASS_OK)
		{
			handleSelectMethodQueryResult(queryFuture, method, project, prevVersion, parserVersion, &newMethod);
		}
		else
		{
			// Handle error.
			const char *message;
			size_t messageLength;
			cass_future_error_message(queryFuture, &message, &messageLength);
			fprintf(stderr, "Unable to get previous project: '%.*s'\n", (int)messageLength, message);
			errno = ENETUNREACH;
		}

		// Statement objects can be freed immediately after being executed.
		cass_statement_free(query);

		// Block until the query has finished and obtain the error code.
		CassError rc = cass_future_error_code(queryFuture);

		if (rc != 0)
		{
			printf("Unable to retrieve previous method: %s\n", cass_error_desc(rc));
			errno = ENETUNREACH;
		}
		cass_future_free(queryFuture);
	}

	// In case the project is new, or the method happens to not be part of an unchanged file,
	// we create a new entry for the method inside the methods table of the database.
	if (newMethod)
	{
		addNewMethod(method, project, parserVersion);
	}
}

void DatabaseHandler::handleSelectMethodQueryResult(CassFuture *queryFuture, MethodIn method, ProjectIn project,
													long long prevVersion, long long parserVersion, bool newMethod)
{
	const CassResult *result = cass_future_get_result(queryFuture);

	// Add matches to result list.
	CassIterator *iterator = cass_iterator_from_result(result);
	while (cass_iterator_next(iterator))
	{
		const CassRow *row = cass_iterator_get_row(iterator);

		long long endVersion = getInt64(row, "endVersionTime");
		if (endVersion == prevVersion)
		{
			// The method is in fact part of an unchanged file,
			// so we update the details regarding the method.
			newMethod = false;
			long long startVersion = getInt64(row, "startVersionTime");
			updateMethod(method, project, startVersion);
		}
	}

	cass_iterator_free(iterator);
	cass_result_free(result);
}

void DatabaseHandler::addNewMethod(MethodIn method, ProjectIn project, long long parserVersion)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(insertMethod);

	// Bind the variables in the statement.
	CassUuid uuid;
	cass_uuid_from_string(Utility::hashToUUIDString(method.hash).c_str(), &uuid);
	cass_statement_bind_uuid_by_name(query, "method_hash", uuid);
	cass_statement_bind_int64_by_name(query, "projectID", project.projectID);
	cass_statement_bind_int64_by_name(query, "startversiontime", project.version);
	cass_statement_bind_string_by_name(query, "file", method.fileLocation.c_str());
	cass_statement_bind_string_by_name(query, "startversionhash", project.versionHash.c_str());
	cass_statement_bind_int64_by_name(query, "endversiontime", project.version);
	cass_statement_bind_string_by_name(query, "endversionhash", project.versionHash.c_str());
	cass_statement_bind_string_by_name(query, "name", method.methodName.c_str());
	cass_statement_bind_int32_by_name(query, "lineNumber", method.lineNumber);
	cass_statement_bind_int64_by_name(query, "parserversion", parserVersion);

	int size = method.authors.size();
	CassCollection *authors = cass_collection_new(CASS_COLLECTION_TYPE_SET, size);

	// For each author of the method, add an entry to the table method_by_author.
	for (int i = 0; i < size; i++)
	{
		CassUuid authorID = createAuthorIfNotExists(method.authors[i]);
		cass_collection_append_uuid(authors, authorID);
		addMethodByAuthor(authorID, method, project);
	}
	cass_statement_bind_collection_by_name(query, "authors", authors);

	cass_collection_free(authors);

	CassFuture *queryFuture = cass_session_execute(connection, query);

	// Free statement objects immediately after being executed.
	cass_statement_free(query);

	// Block until the query has finished and obtain the error code.
	CassError rc = cass_future_error_code(queryFuture);
	if (rc != 0)
	{
		// Handle error.
		const char *message;
		size_t messageLength;
		cass_future_error_message(queryFuture, &message, &messageLength);
		fprintf(stderr, "Unable to insert new method: '%.*s'\n", (int)messageLength, message);
		errno = ENETUNREACH;
	}

	cass_future_free(queryFuture);
}

void DatabaseHandler::updateMethod(MethodIn method, ProjectIn project, long long startVersion)
{
	errno = 0;

	CassStatement *query = cass_prepared_bind(updateMethods);

	// Bind the variables in the statement.
	CassUuid uuid;
	cass_uuid_from_string(Utility::hashToUUIDString(method.hash).c_str(), &uuid);
	cass_statement_bind_uuid_by_name(query, "method_hash", uuid);
	cass_statement_bind_int64_by_name(query, "projectid", project.projectID);
	cass_statement_bind_string_by_name(query, "file", method.fileLocation.c_str());
	cass_statement_bind_int64_by_name(query, "startversiontime", startVersion);
	cass_statement_bind_int64_by_name(query, "endversiontime", project.version);
	cass_statement_bind_string_by_name(query, "endversionhash", project.versionHash.c_str());
	cass_statement_bind_string_by_name(query, "name", method.methodName.c_str());
	cass_statement_bind_int32_by_name(query, "lineNumber", method.lineNumber);

	int size = method.authors.size();

	CassCollection *authors = cass_collection_new(CASS_COLLECTION_TYPE_SET, size);

	for (int i = 0; i < size; i++)
	{
		CassUuid authorID = createAuthorIfNotExists(method.authors[i]);
		cass_collection_append_uuid(authors, authorID);
		addMethodByAuthor(authorID, method, project);
	}

	cass_statement_bind_collection_by_name(query, "authors", authors);
	cass_collection_free(authors);

	CassFuture *queryFuture = cass_session_execute(connection, query);

	// Statement objects can be freed immediately after being executed.
	cass_statement_free(query);

	// This will block until the query has finished.
	CassError rc = cass_future_error_code(queryFuture);
	if (rc != 0)
	{
		// Handle error.
		const char *message;
		size_t messageLength;
		cass_future_error_message(queryFuture, &message, &messageLength);
		fprintf(stderr, "Unable to update method: '%.*s'\n", (int)messageLength, message);
		errno = ENETUNREACH;
	}

	cass_future_free(queryFuture);
}

std::vector<Hash> DatabaseHandler::updateUnchangedFiles(std::vector<Hash> hashes, std::vector<std::string> files,
														ProjectIn project, long long prevVersion)
{
	errno = 0;

	CassFuture *queryFuture = executeSelectUnchangedMethodsQuery(hashes, files, project);

	std::vector<Hash> resultHashes;

	if (cass_future_error_code(queryFuture) == CASS_OK)
	{
		resultHashes = handleSelectUnchangedMethodsResult(queryFuture, project, prevVersion);
	}
	else
	{
		// An error occurred, which is handled below.
		const char *message;
		size_t messageLength;
		cass_future_error_message(queryFuture, &message, &messageLength);
		fprintf(stderr, "Unable to retrieve unchanged methods: '%.*s'\n", (int)messageLength, message);
		errno = ENETUNREACH;
	}

	cass_future_free(queryFuture);

	return resultHashes;
}

CassFuture *DatabaseHandler::executeSelectUnchangedMethodsQuery(std::vector<Hash> hashes,
																std::vector<std::string> files, ProjectIn project)
{
	CassStatement *query = cass_prepared_bind(selectUnchangedMethods);
	cass_statement_bind_int64_by_name(query, "projectid", project.projectID);

	CassCollection *hashesCollection = cass_collection_new(CASS_COLLECTION_TYPE_LIST, hashes.size());
	for (int i = 0; i < hashes.size(); i++)
	{
		CassUuid hash;
		cass_uuid_from_string(Utility::hashToUUIDString(hashes[i]).c_str(), &hash);
		cass_collection_append_uuid(hashesCollection, hash);
	}

	cass_statement_bind_collection(query, 0, hashesCollection);
	cass_collection_free(hashesCollection);

	CassCollection *filesCollection = cass_collection_new(CASS_COLLECTION_TYPE_LIST, files.size());
	for (int i = 0; i < files.size(); i++)
	{
		cass_collection_append_string(filesCollection, files[i].c_str());
	}

	cass_statement_bind_collection(query, 2, filesCollection);
	cass_collection_free(filesCollection);

	CassFuture *queryFuture = cass_session_execute(connection, query);

	// Statement objects can be freed immediately after being executed.
	cass_statement_free(query);

	return queryFuture;
}

std::vector<Hash> DatabaseHandler::handleSelectUnchangedMethodsResult(CassFuture *queryFuture, ProjectIn project,
																	  long long prevVersion)
{
	std::vector<Hash> hashes;
	const CassResult *result = cass_future_get_result(queryFuture);
	CassIterator *iterator = cass_iterator_from_result(result);

	// Add matches to result list.
	while (cass_iterator_next(iterator))
	{
		const CassRow *row = cass_iterator_get_row(iterator);

		long long endVersion = getInt64(row, "endVersionTime");

		if (endVersion == prevVersion)
		{
			long long startVersion = getInt64(row, "startversiontime");
			MethodIn method;
			method.hash = Utility::uuidStringToHash(getUUID(row, "method_hash"));
			method.fileLocation = getString(row, "file");
			updateMethod(method, project, startVersion);
			hashes.push_back(method.hash);
		}
	}

	cass_iterator_free(iterator);
	cass_result_free(result);

	return hashes;
}

void DatabaseHandler::addMethodByAuthor(CassUuid authorID, MethodIn method, ProjectIn project)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(insertMethodByAuthor);

	// Bind the variables in the statement.
	cass_statement_bind_uuid_by_name(query, "authorID", authorID);

	CassUuid uuid;
	cass_uuid_from_string(Utility::hashToUUIDString(method.hash).c_str(), &uuid);
	cass_statement_bind_uuid_by_name(query, "hash", uuid);
	cass_statement_bind_int64_by_name(query, "projectID", project.projectID);
	cass_statement_bind_string_by_name(query, "file", method.fileLocation.c_str());
	cass_statement_bind_int64_by_name(query, "startversiontime", project.version);

	CassFuture *queryFuture = cass_session_execute(connection, query);

	// Statement objects can be freed immediately after being executed.
	cass_statement_free(query);

	// This will block until the query has finished.
	CassError rc = cass_future_error_code(queryFuture);

	if (rc != 0)
	{
		// An error occurred which is handled below.
		const char *message;
		size_t messageLength;
		cass_future_error_message(queryFuture, &message, &messageLength);
		fprintf(stderr, "Unable to relate the author to the method: '%.*s'\n", (int)messageLength, message);
		errno = ENETUNREACH;
	}

	cass_future_free(queryFuture);
}
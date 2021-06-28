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

ProjectOut DatabaseHandler::searchForProject(ProjectID projectID, Version version)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(selectProject);

	// Bind the variables in the statement.
	cass_statement_bind_int64_by_name(query, "projectID", projectID);
	cass_statement_bind_int64_by_name(query, "versiontime", version);

	CassFuture *resultFuture = cass_session_execute(connection, query);
	ProjectOut project;
	if (cass_future_error_code(resultFuture) == CASS_OK)
	{
		const CassResult *result = cass_future_get_result(resultFuture);
		if (cass_result_row_count(result) >= 1)
		{
			const CassRow *row = cass_result_first_row(result);
			project = getProject(row);
		}
		else
		{
			// No project with the provided projectID and version exists.
			errno = ERANGE;
		}

		cass_result_free(result);
	}
	else
	{
		// An error occurred which is handled below.
		const char *message;
		size_t messageLength;
		cass_future_error_message(resultFuture, &message, &messageLength);
		fprintf(stderr, "Unable to search for project: '%.*s'\n", (int)messageLength, message);
		errno = ENETUNREACH;
	}

	cass_statement_free(query);
	cass_future_free(resultFuture);

	return project;
}

std::vector<MethodOut> DatabaseHandler::hashToMethods(std::string hash)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(selectMethods);

	// To bind the hash as a UUID in the query, we have to convert it to a UUID first.
	CassUuid uuid;
	cass_uuid_from_string(Utility::hashToUUIDString(hash).c_str(), &uuid);
	cass_statement_bind_uuid_by_name(query, "method_hash", uuid);

	CassFuture *resultFuture = cass_session_execute(connection, query);

	std::vector<MethodOut> methods;

	if (cass_future_error_code(resultFuture) == CASS_OK)
	{
		const CassResult *result = cass_future_get_result(resultFuture);

		// Add the methods found as the result of the query.
		CassIterator *iterator = cass_iterator_from_result(result);
		while (cass_iterator_next(iterator))
		{
			const CassRow *row = cass_iterator_get_row(iterator);
			methods.push_back(getMethod(row));
		}

		cass_iterator_free(iterator);
		cass_result_free(result);
	}
	else
	{
		// An error occurred which is handled below.
		const char *message;
		size_t messageLength;
		cass_future_error_message(resultFuture, &message, &messageLength);
		fprintf(stderr, "Unable to add hash to the method: '%.*s'\n", (int)messageLength, message);
		errno = ENETUNREACH;
	}

	cass_statement_free(query);
	cass_future_free(resultFuture);

	return methods;
}

ProjectOut DatabaseHandler::prevProject(ProjectID projectID)
{
	CassStatement *query = cass_prepared_bind(selectPrevProject);
	cass_statement_bind_int64_by_name(query, "projectID", projectID);

	CassFuture *resultFuture = cass_session_execute(connection, query);

	ProjectOut project;
	project.projectID = -1;

	if (cass_future_error_code(resultFuture) == CASS_OK)
	{
		const CassResult *result = cass_future_get_result(resultFuture);
		if (cass_result_row_count(result) >= 1)
		{
			const CassRow *row = cass_result_first_row(result);
			project = getProject(row);
		}

		cass_result_free(result);
	}
	else
	{
		// An error occurred which is handled below.
		const char *message;
		size_t messageLength;
		cass_future_error_message(resultFuture, &message, &messageLength);
		fprintf(stderr, "Unable to retrieve previous version of project: '%.*s'\n", (int)messageLength, message);
		errno = ENETUNREACH;
	}

	cass_statement_free(query);
	cass_future_free(resultFuture);

	return project;
}

std::vector<MethodID> DatabaseHandler::authorToMethods(std::string authorID)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(selectMethodByAuthor);

	CassUuid uuid;
	cass_uuid_from_string(authorID.c_str(), &uuid);
	cass_statement_bind_uuid_by_name(query, "authorid", uuid);

	CassFuture *resultFuture = cass_session_execute(connection, query);

	std::vector<MethodID> methods;

	if (cass_future_error_code(resultFuture) == CASS_OK)
	{
		const CassResult *result = cass_future_get_result(resultFuture);
		CassIterator *iterator = cass_iterator_from_result(result);

		// Add matches to result list.
		while (cass_iterator_next(iterator))
		{
			const CassRow *row = cass_iterator_get_row(iterator);
			methods.push_back(getMethodID(row));
		}

		cass_iterator_free(iterator);
		cass_result_free(result);
	}
	else
	{
		// An error occurred which is handled below.
		const char *message;
		size_t messageLength;
		cass_future_error_message(resultFuture, &message, &messageLength);
		fprintf(stderr, "Unable to obtain the methods by the author: '%.*s'\n", (int)messageLength, message);
		errno = ENETUNREACH;
	}

	cass_statement_free(query);
	cass_future_free(resultFuture);

	return methods;
}

Author DatabaseHandler::idToAuthor(std::string id)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(selectAuthorByID);

	CassUuid uuid;
	cass_uuid_from_string(id.c_str(), &uuid);
	cass_statement_bind_uuid_by_name(query, "authorid", uuid);

	CassFuture *resultFuture = cass_session_execute(connection, query);

	Author author("", "");

	if (cass_future_error_code(resultFuture) == CASS_OK)
	{
		const CassResult *result = cass_future_get_result(resultFuture);

		if (cass_result_row_count(result) >= 1)
		{
			const CassRow *row = cass_result_first_row(result);
			author = Author(getString(row, "name"), getString(row, "mail"));
		}

		cass_result_free(result);
	}
	else
	{
		// An error occurred which is handled below.
		const char *message;
		size_t messageLength;
		cass_future_error_message(resultFuture, &message, &messageLength);
		fprintf(stderr, "Unable to retrieve the author: '%.*s'\n", (int)messageLength, message);
		errno = ENETUNREACH;
	}

	cass_statement_free(query);
	cass_future_free(resultFuture);

	return author;
}

CassUuid DatabaseHandler::createAuthorIfNotExists(Author author)
{
	CassUuid authorID;
	cass_uuid_from_string(Utility::hashToUUIDString(author.id).c_str(), &authorID);

	CassStatement *query = cass_prepared_bind(insertAuthorByID);

	// Bind the variables in the statement.
	cass_statement_bind_uuid_by_name(query, "authorID", authorID);
	cass_statement_bind_string_by_name(query, "name", author.name.c_str());
	cass_statement_bind_string_by_name(query, "mail", author.mail.c_str());

	cass_session_execute(connection, query);

	cass_statement_free(query);

	return authorID;
}

ProjectOut DatabaseHandler::getProject(const CassRow *row)
{
	ProjectOut project;

	// Retrieve the values of the variables in the row.
	project.projectID = getInt64(row, "projectID");
	project.version = getInt64(row, "versiontime");
	project.versionHash = getString(row, "versionHash");
	project.license = getString(row, "license");
	project.name = getString(row, "name");
	project.url = getString(row, "url");
	project.ownerID = getUUID(row, "ownerid");
	project.parserVersion = getInt64(row, "parserversion");

	const CassValue *set = cass_row_get_column_by_name(row, "hashes");
	CassIterator *iterator = cass_iterator_from_collection(set);

	if (iterator)
	{
		// Retrieve the hashes one by one.
		while (cass_iterator_next(iterator))
		{
			char hash[CASS_UUID_STRING_LENGTH];
			CassUuid uuid;
			const CassValue *id = cass_iterator_get_value(iterator);
			cass_value_get_uuid(id, &uuid);
			cass_uuid_string(uuid, hash);
			project.hashes.push_back(Utility::uuidStringToHash(hash));
		}
	}

	cass_iterator_free(iterator);

	return project;
}

MethodOut DatabaseHandler::getMethod(const CassRow *row)
{
	MethodOut method;

	// Retrieve the values of the variables in the row.
	method.hash = Utility::uuidStringToHash(getUUID(row, "method_hash"));
	method.methodName = getString(row, "name");
	method.fileLocation = getString(row, "file");
	method.lineNumber = getInt32(row, "lineNumber");
	method.projectID = getInt64(row, "projectID");
	method.startVersion = getInt64(row, "startversiontime");
	method.startVersionHash = getString(row, "startversionhash");
	method.endVersion = getInt64(row, "endversiontime");
	method.endVersionHash = getString(row, "endversionhash");
	method.parserVersion = getInt64(row, "parserversion");

	const CassValue *set = cass_row_get_column_by_name(row, "authors");
	CassIterator *iterator = cass_iterator_from_collection(set);
	if (iterator)
	{
		// Retrieve the authors one by one.
		while (cass_iterator_next(iterator))
		{
			char authorID[CASS_UUID_STRING_LENGTH];
			CassUuid uuid;
			const CassValue *id = cass_iterator_get_value(iterator);
			cass_value_get_uuid(id, &uuid);
			cass_uuid_string(uuid, authorID);
			method.authorIDs.push_back(authorID);
		}
	}

	cass_iterator_free(iterator);

	return method;
}

MethodID DatabaseHandler::getMethodID(const CassRow *row)
{
	MethodID method;

	// Retrieve the values of the variables in the row.
	method.hash = Utility::uuidStringToHash(getUUID(row, "hash"));
	method.projectID = getInt64(row, "projectid");
	method.startVersion = getInt64(row, "startversiontime");
	method.fileLocation = getString(row, "file");

	return method;
}

std::string DatabaseHandler::getString(const CassRow *row, const char *column)
{
	const char *result;
	size_t len;
	const CassValue *value = cass_row_get_column_by_name(row, column);
	cass_value_get_string(value, &result, &len);
	return std::string(result, len);
}

int DatabaseHandler::getInt32(const CassRow *row, const char *column)
{
	cass_int32_t result;
	const CassValue *value = cass_row_get_column_by_name(row, column);
	cass_value_get_int32(value, &result);
	return result;
}

long long DatabaseHandler::getInt64(const CassRow *row, const char *column)
{
	cass_int64_t result;
	const CassValue *value = cass_row_get_column_by_name(row, column);
	cass_value_get_int64(value, &result);
	return result;
}

std::string DatabaseHandler::getUUID(const CassRow *row, const char *column)
{
	char result[CASS_UUID_STRING_LENGTH];
	CassUuid authorID;
	const CassValue *value = cass_row_get_column_by_name(row, column);
	cass_value_get_uuid(value, &authorID);
	cass_uuid_string(authorID, result);
	return result;
}

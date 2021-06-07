/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "DatabaseHandler.h"
#include <iostream>

void DatabaseHandler::connect(std::string ip, int port)
{
	errno = 0;
	CassFuture *connectFuture = NULL;
	CassCluster *cluster = cass_cluster_new();
	connection = cass_session_new();

	// Add contact points.
	cass_cluster_set_contact_points(cluster, ip.c_str());
	cass_cluster_set_port(cluster, port);
	cass_cluster_set_protocol_version(cluster, CASS_PROTOCOL_VERSION_V3);
	cass_cluster_set_consistency(cluster, CASS_CONSISTENCY_QUORUM);
	cass_cluster_set_num_threads_io(cluster, MAX_THREADS);

	// Provide the cluster object as configuration to connect the session.
	connectFuture = cass_session_connect_keyspace(connection, cluster, "projectdata");

	CassError rc = cass_future_error_code(connectFuture);

	if (rc != CASS_OK)
	{
		// Display connection error message.
		const char *message;
		size_t messageLength;
		cass_future_error_message(connectFuture, &message, &messageLength);
		fprintf(stderr, "Connect error: '%.*s'\n", (int)messageLength, message);
		errno = ENETUNREACH;
	}
	setPreparedStatements();
}

void DatabaseHandler::setPreparedStatements()
{
	// Selects all methods with a given hash.
	CassFuture *prepareFuture =
		cass_session_prepare(connection, "SELECT * FROM projectdata.methods WHERE method_hash = ?");
	CassError rc = cass_future_error_code(prepareFuture);
	selectMethod = cass_future_get_prepared(prepareFuture);

	// Selects all projects with a given projectID and version.
	prepareFuture =
		cass_session_prepare(connection, "SELECT * FROM projectdata.projects WHERE projectID = ? AND version = ?");
	rc = cass_future_error_code(prepareFuture);
	selectProject = cass_future_get_prepared(prepareFuture);

	// Inserts a project into the database.
	prepareFuture = cass_session_prepare(
		connection,
		"INSERT INTO projectdata.projects (projectID, version, license, name, url, ownerid) VALUES (?, ?, ?, ?, ?, ?)");
	rc = cass_future_error_code(prepareFuture);
	insertProject = cass_future_get_prepared(prepareFuture);

	// Inserts a method into the database
	prepareFuture =
		cass_session_prepare(connection, "INSERT INTO projectdata.methods (method_hash, version, projectID, name, "
										 "file, lineNumber ,authors) VALUES (?, ?, ?, ?, ?, ?, ?)");
	rc = cass_future_error_code(prepareFuture);
	insertMethod = cass_future_get_prepared(prepareFuture);

	// Inserts a method by author.
	prepareFuture = cass_session_prepare(
		connection,
		"INSERT INTO projectdata.method_by_author (authorID, hash, version, projectID) VALUES (?, ?, ?, ?)");
	rc = cass_future_error_code(prepareFuture);
	insertMethodByAuthor = cass_future_get_prepared(prepareFuture);

	// Selects a method by a given authorid.
	prepareFuture = cass_session_prepare(connection, "SELECT * FROM projectdata.method_by_author WHERE authorid = ?");
	rc = cass_future_error_code(prepareFuture);
	selectMethodByAuthor = cass_future_get_prepared(prepareFuture);

	// Selects the authorID corresponding to given name and mail.
	prepareFuture =
		cass_session_prepare(connection, "SELECT authorID FROM projectdata.id_by_author WHERE name = ? AND mail = ?");
	rc = cass_future_error_code(prepareFuture);
	selectIdByAuthor = cass_future_get_prepared(prepareFuture);

	// Inserts a new author into the id_by_author table. Also generated the id for this author.
	prepareFuture = cass_session_prepare(
		connection, "INSERT INTO projectdata.id_by_author (authorID, name, mail) VALUES (uuid(), ?, ?)");
	rc = cass_future_error_code(prepareFuture);
	insertIdByAuthor = cass_future_get_prepared(prepareFuture);

	// Inserts an author into the author_by_id table.
	prepareFuture = cass_session_prepare(
		connection, "INSERT INTO projectdata.author_by_id (authorID, name, mail) VALUES (?, ?, ?)");
	rc = cass_future_error_code(prepareFuture);
	insertAuthorById = cass_future_get_prepared(prepareFuture);

	// Inserts an author into the author_by_id table.
	prepareFuture = cass_session_prepare(connection, "SELECT * FROM projectdata.author_by_id WHERE authorid = ?");
	rc = cass_future_error_code(prepareFuture);
	selectAuthorById = cass_future_get_prepared(prepareFuture);

	cass_future_free(prepareFuture);
}

std::vector<ProjectOut> DatabaseHandler::searchForProject(ProjectID projectID, Version version)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(selectProject);
	cass_statement_bind_int64_by_name(query, "projectID", projectID);
	cass_statement_bind_int64_by_name(query, "version", version);

	CassFuture *resultFuture = cass_session_execute(connection, query);
	std::vector<ProjectOut> projects = {};
	if (cass_future_error_code(resultFuture) == CASS_OK)
	{
		const CassResult *result = cass_future_get_result(resultFuture);
		CassIterator *iterator = cass_iterator_from_result(result);

		// Add matches to result list.
		while(cass_iterator_next(iterator))
		{
			const CassRow *row = cass_iterator_get_row(iterator);
			projects.push_back(getProject(row));
		}

		cass_iterator_free(iterator);
		cass_result_free(result);
	}
	else
	{
		// Handle error.
		const char *message;
		size_t messageLength;
		cass_future_error_message(resultFuture, &message, &messageLength);
		fprintf(stderr, "Unable to run query: '%.*s'\n", (int)messageLength, message);
		errno = ENETUNREACH;
	}

	cass_statement_free(query);
	cass_future_free(resultFuture);

	return projects;
}

std::vector<MethodOut> DatabaseHandler::hashToMethods(std::string hash)
{
	errno = 0;
	CassUuid uuid;
	cass_uuid_from_string(hash.c_str(), &uuid);
	CassStatement* query = cass_prepared_bind(selectMethod);

	cass_statement_bind_uuid_by_name(query, "method_hash", uuid);

	CassFuture *resultFuture = cass_session_execute(connection, query);

	std::vector<MethodOut> methods;

	if (cass_future_error_code(resultFuture) == CASS_OK)
	{
		const CassResult *result = cass_future_get_result(resultFuture);

		CassIterator *iterator = cass_iterator_from_result(result);

		// Add matches to result list.
		while(cass_iterator_next(iterator))
		{
			const CassRow *row = cass_iterator_get_row(iterator);
			methods.push_back(getMethod(row));
		}

		cass_iterator_free(iterator);

		cass_result_free(result);

	}
	else
	{
		// Handle error.
		const char *message;
		size_t messageLength;
		cass_future_error_message(resultFuture, &message, &messageLength);
		fprintf(stderr, "Unable to run query: '%.*s'\n", (int)messageLength, message);
		errno = ENETUNREACH;
	}

	cass_statement_free(query);
	cass_future_free(resultFuture);

	return methods;
}

void DatabaseHandler::addProject(ProjectIn project)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(insertProject);

	cass_statement_bind_int64_by_name(query, "projectID", project.projectID);

	cass_statement_bind_int64_by_name(query, "version", project.version);

	cass_statement_bind_string_by_name(query, "license", project.license.c_str());

	cass_statement_bind_string_by_name(query, "name", project.name.c_str());

	cass_statement_bind_string_by_name(query, "url", project.url.c_str());

	cass_statement_bind_uuid_by_name(query, "ownerid", getAuthorId(project.owner));

	CassFuture *queryFuture = cass_session_execute(connection, query);

	// Statement objects can be freed immediately after being executed.
	cass_statement_free(query);

	// This will block until the query has finished.
	CassError rc = cass_future_error_code(queryFuture);

	if (rc != 0)
	{
		printf("Query result: %s\n", cass_error_desc(rc));
		errno = ENETUNREACH;
	}

	cass_future_free(queryFuture);
}

void DatabaseHandler::addMethod(MethodIn method, ProjectIn project)
{
	errno = 0;
	CassUuid uuid;
	cass_uuid_from_string(method.hash.c_str(), &uuid);
	CassStatement *query = cass_prepared_bind(insertMethod);

	cass_statement_bind_uuid_by_name(query, "method_hash", uuid);

	cass_statement_bind_int64_by_name(query, "version", project.version);

	cass_statement_bind_int64_by_name(query, "projectID", project.projectID);

	cass_statement_bind_string_by_name(query, "name", method.methodName.c_str());

	cass_statement_bind_string_by_name(query, "file", method.fileLocation.c_str());

	cass_statement_bind_int32_by_name(query, "lineNumber", method.lineNumber);

	int size = method.authors.size();

	CassCollection *authors = cass_collection_new(CASS_COLLECTION_TYPE_SET, size);

	for (int i = 0; i < size; i++)
	{
		CassUuid authorID = getAuthorId(method.authors[i]);
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
		printf("Query result: %s\n", cass_error_desc(rc));
		errno = ENETUNREACH;
	}

	cass_future_free(queryFuture);
}

void DatabaseHandler::addMethodByAuthor(CassUuid authorID, MethodIn method, ProjectIn project)
{
	errno = 0;
	CassUuid uuid;
	cass_uuid_from_string(method.hash.c_str(), &uuid);
	CassStatement *query = cass_prepared_bind(insertMethodByAuthor);

	cass_statement_bind_uuid_by_name(query, "authorID", authorID);

	cass_statement_bind_uuid_by_name(query, "hash", uuid);

	cass_statement_bind_int64_by_name(query, "version", project.version);

	cass_statement_bind_int64_by_name(query, "projectID", project.projectID);

	CassFuture *queryFuture = cass_session_execute(connection, query);

	// Statement objects can be freed immediately after being executed.
	cass_statement_free(query);

	// This will block until the query has finished.
	CassError rc = cass_future_error_code(queryFuture);

	if (rc != 0)
	{
		printf("Query result: %s\n", cass_error_desc(rc));
		errno = ENETUNREACH;
	}

	cass_future_free(queryFuture);
}

std::vector<MethodId> DatabaseHandler::authorToMethods(std::string authorId)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(selectMethodByAuthor);

	CassUuid uuid;
	cass_uuid_from_string(authorId.c_str(), &uuid);
	cass_statement_bind_uuid_by_name(query, "authorid", uuid);

	CassFuture *resultFuture = cass_session_execute(connection, query);

	std::vector<MethodId> methods;

	if (cass_future_error_code(resultFuture) == CASS_OK)
	{
		const CassResult *result = cass_future_get_result(resultFuture);

		CassIterator *iterator = cass_iterator_from_result(result);

		// Add matches to result list.
		while (cass_iterator_next(iterator))
		{
			const CassRow *row = cass_iterator_get_row(iterator);
			methods.push_back(getMethodId(row));
		}

		cass_iterator_free(iterator);

		cass_result_free(result);
	}
	else
	{
		// Handle error.
		const char *message;
		size_t messageLength;
		cass_future_error_message(resultFuture, &message, &messageLength);
		fprintf(stderr, "Unable to run query: '%.*s'\n", (int)messageLength, message);
		errno = ENETUNREACH;
	}

	cass_statement_free(query);
	cass_future_free(resultFuture);

	return methods;
}

Author DatabaseHandler::idToAuthor(std::string id)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(selectAuthorById);

	CassUuid uuid;
	cass_uuid_from_string(id.c_str(), &uuid);
	cass_statement_bind_uuid_by_name(query, "authorid", uuid);

	CassFuture *resultFuture = cass_session_execute(connection, query);

	Author author;

	if (cass_future_error_code(resultFuture) == CASS_OK)
	{
		const CassResult *result = cass_future_get_result(resultFuture);

		if (cass_result_row_count(result) >= 1)
		{
			const CassRow *row = cass_result_first_row(result);

			author.name = getString(row, "name");
			author.mail = getString(row, "mail");
		}

		cass_result_free(result);
	}
	else
	{
		// Handle error.
		const char *message;
		size_t messageLength;
		cass_future_error_message(resultFuture, &message, &messageLength);
		fprintf(stderr, "Unable to run query: '%.*s'\n", (int)messageLength, message);
		errno = ENETUNREACH;
	}

	cass_statement_free(query);
	cass_future_free(resultFuture);

	return author;
}

CassUuid DatabaseHandler::getAuthorId(Author author)
{
	CassUuid authorID = retrieveAuthorId(author);
	if (errno != 0)
	{
		authorID = createAuthor(author);
		errno = 0;
	}
	return authorID;
}

std::string DatabaseHandler::authorToId(Author author)
{
	CassUuid authorID = retrieveAuthorId(author);

	if (errno == 0)
	{
		char result[CASS_UUID_STRING_LENGTH];
		cass_uuid_string(authorID, result);
		return result;
	}
	return "";
}

CassUuid DatabaseHandler::retrieveAuthorId(Author author)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(selectIdByAuthor);

	cass_statement_bind_string_by_name(query, "name", author.name.c_str());
	cass_statement_bind_string_by_name(query, "mail", author.mail.c_str());
	CassFuture *queryFuture = cass_session_execute(connection, query);

	CassUuid authorId;

	if (cass_future_error_code(queryFuture) == CASS_OK)
	{
		const CassResult *result = cass_future_get_result(queryFuture);

		if (cass_result_row_count(result) >= 1)
		{
			const CassRow *row = cass_result_first_row(result);
			const CassValue *id = cass_row_get_column(row, 0);
			cass_value_get_uuid(id, &authorId);
		}
		else
		{
			errno = ERANGE;
		}

		cass_result_free(result);
	}
	else
	{
		// Handle error.
		const char *message;
		size_t messageLength;
		cass_future_error_message(queryFuture, &message, &messageLength);
		fprintf(stderr, "Unable to run query: '%.*s'\n", (int)messageLength, message);
		errno = ENETUNREACH;
	}

	cass_statement_free(query);
	cass_future_free(queryFuture);

	return authorId;
}

CassUuid DatabaseHandler::createAuthor(Author author)
{
	CassStatement *insertQuery = cass_prepared_bind(insertIdByAuthor);

	cass_statement_bind_string_by_name(insertQuery, "name", author.name.c_str());
	cass_statement_bind_string_by_name(insertQuery, "mail", author.mail.c_str());

	CassFuture *future = cass_session_execute(connection, insertQuery);

	cass_future_wait(future);

	cass_statement_free(insertQuery);

	cass_future_free(future);

	CassUuid authorId = retrieveAuthorId(author);

	CassStatement *insertQuery2 = cass_prepared_bind(insertAuthorById);

	cass_statement_bind_uuid_by_name(insertQuery2, "authorID", authorId);
	cass_statement_bind_string_by_name(insertQuery2, "name", author.name.c_str());
	cass_statement_bind_string_by_name(insertQuery2, "mail", author.mail.c_str());
	cass_session_execute(connection, insertQuery2);

	cass_statement_free(insertQuery2);

	return authorId;
}

ProjectOut DatabaseHandler::getProject(const CassRow *row)
{
	ProjectOut project;
	project.projectID = getInt64(row, "projectID");
	project.version = getInt64(row, "version");
	project.license = getString(row, "license");
	project.name = getString(row, "name");
	project.url = getString(row, "url");
	project.ownerID = getUUID(row, "ownerid");

	return project;
}

MethodOut DatabaseHandler::getMethod(const CassRow *row)
{
	MethodOut method;

	method.hash = getUUID(row, "method_hash");
	method.methodName = getString(row, "name");
	method.fileLocation = getString(row, "file");

	method.lineNumber = getInt32(row, "lineNumber");
	method.projectID = getInt64(row, "projectID");
	method.version = getInt64(row, "version");

	const CassValue *set = cass_row_get_column(row, 3);
	CassIterator *iterator = cass_iterator_from_collection(set);

	if (iterator)
	{
		while (cass_iterator_next(iterator))
		{
			char authorId[CASS_UUID_STRING_LENGTH];
			CassUuid authorID;
			const CassValue *id = cass_iterator_get_value(iterator);
			cass_value_get_uuid(id, &authorID);
			cass_uuid_string(authorID, authorId);
			method.authorIDs.push_back(authorId);
		}
	}

	cass_iterator_free(iterator);

	return method;
}

MethodId DatabaseHandler::getMethodId(const CassRow *row)
{
	MethodId method;

	method.hash = getUUID(row, "method_hash");
	method.projectId = getInt64(row, "projectid");
	method.version = getInt64(row, "version");

	return method;
}


std::string DatabaseHandler::getString(const CassRow* row, const char* column)
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

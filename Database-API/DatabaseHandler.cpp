/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "DatabaseHandler.h"
#include "Utility.h"
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
	selectMethods = prepareStatement("SELECT * FROM projectdata.methods WHERE method_hash = ?");

	// Selects all projects with a given projectID and version.
	selectProject = prepareStatement("SELECT * FROM projectdata.projects WHERE projectID = ? AND versiontime = ?");
	
	// Selects the previous version of some project.
	selectPrevProject = prepareStatement("SELECT * FROM projectdata.projects WHERE projectID = ? LIMIT 1");

	// Inserts a project into the database.
	insertProject = prepareStatement("INSERT INTO projectdata.projects (projectID, versiontime, versionhash, license, name, url, ownerid, hashes, parserversion) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");

	// Adds hashes to a project in the database.
	addHashesToProject = prepareStatement("UPDATE projectdata.projects SET hashes = hashes + ? WHERE projectID = ? AND versiontime = ?");

	// Inserts a method into the database.
	insertMethod = prepareStatement("INSERT INTO projectdata.methods (method_hash, projectID, startversiontime, file, startversionhash, endversiontime, endversionhash, name, "
									"lineNumber, authors, parserversion) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

	// Updates a method in the database.
	updateMethods = prepareStatement("UPDATE projectdata.methods SET endVersionTime = ?, endVersionHash = ?, name = ?, lineNumber = ?, authors = authors + ?"
									 "WHERE method_hash = ? AND projectID = ? AND file = ? AND startVersionTime = ?");

	// Retrieves a method from the database.
	selectMethod = prepareStatement("SELECT * FROM projectdata.methods WHERE method_hash = ? AND projectID = ? AND file = ?");

	// Retrieves the methods with specified hash and file.
	selectUnchangedMethods = prepareStatement("SELECT method_hash, projectid, file, startversiontime, endversiontime, linenumber, name, startversionhash, endversionhash FROM projectdata.methods WHERE method_hash IN ? AND projectID = ? AND file IN ?");

	// Inserts a method by author.
	insertMethodByAuthor = prepareStatement("INSERT INTO projectdata.method_by_author (authorID, hash, startversiontime, file, projectID) VALUES (?, ?, ?, ?, ?)");

	// Selects a method by a given authorid.
	selectMethodByAuthor = prepareStatement("SELECT * FROM projectdata.method_by_author WHERE authorid = ?");

	// Inserts an author into the author_by_id table.
	insertAuthorById = prepareStatement("INSERT INTO projectdata.author_by_id (authorID, name, mail) VALUES (?, ?, ?) IF NOT EXISTS");

	// Selects an author from the author_by_id table.
	selectAuthorById = prepareStatement("SELECT * FROM projectdata.author_by_id WHERE authorid = ?");
}

const CassPrepared* DatabaseHandler::prepareStatement(std::string query)
{
	CassFuture *prepareFuture =	cass_session_prepare(connection, query.c_str());
	CassError rc = cass_future_error_code(prepareFuture);
	if (rc != 0)
	{
		const char *message;
		size_t messageLength;
		cass_future_error_message(prepareFuture, &message, &messageLength);
		fprintf(stderr, "Unable to prepare query: '%.*s'\n", (int)messageLength, message);
	}
	const CassPrepared *result = cass_future_get_prepared(prepareFuture);
	cass_future_free(prepareFuture);
	return result;
}

ProjectOut DatabaseHandler::searchForProject(ProjectID projectID, Version version)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(selectProject);
	cass_statement_bind_int64_by_name(query, "projectID", projectID);
	cass_statement_bind_int64_by_name(query, "versiontime", version);

	CassFuture *resultFuture = cass_session_execute(connection, query);
	ProjectOut project = {};
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
			errno = ERANGE;
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

	return project;
}

std::vector<MethodOut> DatabaseHandler::hashToMethods(std::string hash)
{
	errno = 0;
	CassStatement* query = cass_prepared_bind(selectMethods);

	CassUuid uuid;
	cass_uuid_from_string(Utility::hashToUuidString(hash).c_str(), &uuid);
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
	cass_statement_bind_int64_by_name(query, "versiontime", project.version);
	cass_statement_bind_string_by_name(query, "versionhash", project.versionHash.c_str());
	cass_statement_bind_string_by_name(query, "license", project.license.c_str());
	cass_statement_bind_string_by_name(query, "name", project.name.c_str());
	cass_statement_bind_string_by_name(query, "url", project.url.c_str());
	cass_statement_bind_uuid_by_name(query, "ownerid", createAuthorIfNotExists(project.owner));
	cass_statement_bind_int64_by_name(query, "parserversion", project.parserVersion);

	int size = project.hashes.size();

	CassCollection *hashes = cass_collection_new(CASS_COLLECTION_TYPE_SET, size);

	// Add the hashes, but no more then HASHES_TO_INSERT_AT_ONCE
	for (int i = 0; i < std::min(HASHES_TO_INSERT_AT_ONCE, size); i++)
	{
		CassUuid hash;
		cass_uuid_from_string(Utility::hashToUuidString(project.hashes[i]).c_str(), &hash);
		cass_collection_append_uuid(hashes, hash);
	}

	cass_statement_bind_collection_by_name(query, "hashes", hashes);

	CassFuture *queryFuture = cass_session_execute(connection, query);

	// Statement objects can be freed immediately after being executed.
	cass_statement_free(query);

	// This will block until the query has finished.
	CassError rc = cass_future_error_code(queryFuture);

	if (rc != 0)
	{
		printf("Query result: %s\n", cass_error_desc(rc));
		std::cout << cass_error_desc(rc) << std::endl;
		errno = ENETUNREACH;
	}

	cass_future_free(queryFuture);

	// Add the extra hashes if necessary
	if (size > HASHES_TO_INSERT_AT_ONCE)
	{
		addHashToProject(project, HASHES_TO_INSERT_AT_ONCE);
	}
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
		cass_future_error_message(resultFuture, &message, &messageLength);
		fprintf(stderr, "Unable to get previous project: '%.*s'\n", (int)messageLength, message);
		errno = ENETUNREACH;
	}

	cass_statement_free(query);
	cass_future_free(resultFuture);

	return project;
}

void DatabaseHandler::addHashToProject(ProjectIn project, int index)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(addHashesToProject);

	cass_statement_bind_int64_by_name(query, "projectID", project.projectID);

	cass_statement_bind_int64_by_name(query, "versiontime", project.version);

	int size = project.hashes.size();

	CassCollection *hashes = cass_collection_new(CASS_COLLECTION_TYPE_SET, size);

	for (int i = index; i < std::min(index + HASHES_TO_INSERT_AT_ONCE, size); i++)
	{
		CassUuid hash;
		cass_uuid_from_string(Utility::hashToUuidString(project.hashes[i]).c_str(), &hash);
		cass_collection_append_uuid(hashes, hash);
	}

	cass_statement_bind_collection_by_name(query, "hashes", hashes);

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

	// Recursively add hashes so long as necessary
	if (size > index + HASHES_TO_INSERT_AT_ONCE)
	{
		addHashToProject(project, index + HASHES_TO_INSERT_AT_ONCE);
	}
}

void DatabaseHandler::addMethod(MethodIn method, ProjectIn project, long long prevVersion, long long parserVersion, bool newProject)
{
	errno = 0;

	bool newMethod = true;
	if (!newProject)
	{
		CassStatement *query = cass_prepared_bind(selectMethod);

		std::string hashUUID = Utility::hashToUuidString(method.hash);

		CassUuid uuid;
		cass_uuid_from_string(hashUUID.c_str(), &uuid);
		cass_statement_bind_uuid_by_name(query, "method_hash", uuid);
		cass_statement_bind_int64_by_name(query, "projectID", project.projectID);
		cass_statement_bind_string_by_name(query, "file", method.fileLocation.c_str());

		CassFuture *queryFuture = cass_session_execute(connection, query);
		if (cass_future_error_code(queryFuture) == CASS_OK)
		{
			const CassResult *result = cass_future_get_result(queryFuture);

			CassIterator *iterator = cass_iterator_from_result(result);

			// Add matches to result list.
			while (cass_iterator_next(iterator))
			{
				const CassRow *row = cass_iterator_get_row(iterator);

				long long endVersion = getInt64(row, "endVersionTime");

				if (endVersion == prevVersion)
				{
					newMethod = false;
					long long startVersion = getInt64(row, "startVersionTime");
					updateMethod(method, project, startVersion);
				}
			}

			cass_iterator_free(iterator);
			cass_result_free(result);
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

		// This will block until the query has finished.
		CassError rc = cass_future_error_code(queryFuture);

		if (rc != 0)
		{
			printf("Unable to retrieve previous method: %s\n", cass_error_desc(rc));
			errno = ENETUNREACH;
		}
		cass_future_free(queryFuture);
	}

	if (newMethod)
	{
		addNewMethod(method, project, parserVersion);
	}	
}

void DatabaseHandler::addNewMethod(MethodIn method, ProjectIn project, long long parserVersion)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(insertMethod);

	CassUuid uuid;
	cass_uuid_from_string(Utility::hashToUuidString(method.hash).c_str(), &uuid);
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
		printf("Unable to insert new method: %s\n", cass_error_desc(rc));
		errno = ENETUNREACH;
	}

	cass_future_free(queryFuture);
}

void DatabaseHandler::updateMethod(MethodIn method, ProjectIn project, long long startVersion)
{
	errno = 0;

	CassStatement *query = cass_prepared_bind(updateMethods);

	CassUuid uuid;
	cass_uuid_from_string(Utility::hashToUuidString(method.hash).c_str(), &uuid);
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
		printf("Unable to update method: %s\n", cass_error_desc(rc));
		errno = ENETUNREACH;
	}

	cass_future_free(queryFuture);
}

std::vector<Hash> DatabaseHandler::updateUnchangedFiles(std::vector<Hash> hashes, std::vector<std::string> files, ProjectIn project, long long prevVersion)
{
	errno = 0;

	CassStatement *query = cass_prepared_bind(selectUnchangedMethods);
	cass_statement_bind_int64_by_name(query, "projectid", project.projectID);

	int size = hashes.size();

	CassCollection *hashesCollection = cass_collection_new(CASS_COLLECTION_TYPE_LIST, size);

	for (int i = 0; i < size; i++)
	{
		CassUuid hash;
		cass_uuid_from_string(Utility::hashToUuidString(hashes[i]).c_str(), &hash);
		cass_collection_append_uuid(hashesCollection, hash);
	}

	cass_statement_bind_collection(query, 0, hashesCollection);

	cass_collection_free(hashesCollection);

	size = files.size();

	CassCollection *filesCollection = cass_collection_new(CASS_COLLECTION_TYPE_LIST, size);

	for (int i = 0; i < size; i++)
	{
		cass_collection_append_string(filesCollection, files[i].c_str());
	}

	cass_statement_bind_collection(query, 2, filesCollection);

	cass_collection_free(filesCollection);

	CassFuture *queryFuture = cass_session_execute(connection, query);

	std::vector<Hash> resultHashes;

	if (cass_future_error_code(queryFuture) == CASS_OK)
	{
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
				resultHashes.push_back(method.hash);
			}
		}

		cass_iterator_free(iterator);
		cass_result_free(result);
	}
	else
	{
		// Handle error.
		const char *message;
		size_t messageLength;
		cass_future_error_message(queryFuture, &message, &messageLength);
		fprintf(stderr, "Unable to get unchanged methods: '%.*s'\n", (int)messageLength, message);
		errno = ENETUNREACH;
	}

	// Statement objects can be freed immediately after being executed.
	cass_statement_free(query);

	// This will block until the query has finished.
	CassError rc = cass_future_error_code(queryFuture);

	if (rc != 0)
	{
		printf("Unable to retrieve unchanged methods: %s\n", cass_error_desc(rc));
		errno = ENETUNREACH;
	}

	cass_future_free(queryFuture);

	return resultHashes;
}

void DatabaseHandler::addMethodByAuthor(CassUuid authorID, MethodIn method, ProjectIn project)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(insertMethodByAuthor);

	cass_statement_bind_uuid_by_name(query, "authorID", authorID);

	CassUuid uuid;
	cass_uuid_from_string(Utility::hashToUuidString(method.hash).c_str(), &uuid);
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

	Author author("","");

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

CassUuid DatabaseHandler::createAuthorIfNotExists(Author author)
{
	CassUuid authorId;
	cass_uuid_from_string(Utility::hashToUuidString(author.id).c_str(), &authorId);

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
		while (cass_iterator_next(iterator))
		{
			char hash[CASS_UUID_STRING_LENGTH];
			CassUuid hashUuid;
			const CassValue *id = cass_iterator_get_value(iterator);
			cass_value_get_uuid(id, &hashUuid);
			cass_uuid_string(hashUuid, hash);
			project.hashes.push_back(Utility::uuidStringToHash(hash));
		}
	}

	cass_iterator_free(iterator);

	return project;
}

MethodOut DatabaseHandler::getMethod(const CassRow *row)
{
	MethodOut method;

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

	method.hash = Utility::uuidStringToHash(getUUID(row, "hash"));
	method.projectId = getInt64(row, "projectid");
	method.startVersion = getInt64(row, "startversiontime");
	method.fileLocation = getString(row, "file");

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

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

	std::cout << "Connecting to the database." << std::endl;

	// Provide the cluster object as configuration to connect the session.
	connectFuture = cass_session_connect_keyspace(connection, cluster, "projectdata");

	CassError rc = cass_future_error_code(connectFuture);

	if (rc != CASS_OK)
	{
		std::cout << "Could not connect to the database." << std::endl;
		std::cout << "Retrying after 45 seconds have elapsed.." << std::endl;
		usleep(45000000);
		std::cout << "Retrying now." << std::endl;

		connectFuture = cass_session_connect_keyspace(connection, cluster, "projectdata");

		CassError rc = cass_future_error_code(connectFuture);

		if (rc != CASS_OK)
		{
			// An error occurred, display connection error message.
			const char *message;
			size_t messageLength;
			cass_future_error_message(connectFuture, &message, &messageLength);
			fprintf(stderr, "Connect error: '%.*s'\n", (int)messageLength, message);
			errno = ENETUNREACH;
		}
	}
	setPreparedStatements();
}

void DatabaseHandler::setPreparedStatements()
{
	// Prepare query used to select all methods with a given hash.
	selectMethods = prepareStatement("SELECT * FROM projectdata.methods WHERE method_hash = ?");

	// Prepare query used to select all projects with a given projectID and version(time).
	selectProject = prepareStatement("SELECT * FROM projectdata.projects WHERE projectID = ? AND versiontime = ?");
	
	// Prepare query used to select the previous/latest version with the given projectID.
	selectPrevProject = prepareStatement("SELECT * FROM projectdata.projects WHERE projectID = ? LIMIT 1");

	// Prepare query used to insert a project into the database.
	insertProject = prepareStatement("INSERT INTO projectdata.projects (projectID, versiontime, versionhash, license, "
									 "name, url, ownerID, hashes, parserversion) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");

	// Prepare query used to add hashes to a project in the database.
	addHashesToProject =
		prepareStatement("UPDATE projectdata.projects SET hashes = hashes + ? WHERE projectID = ? AND versiontime = ?");

	// Prepare query used to insert a method into the database.
	insertMethod = prepareStatement("INSERT INTO projectdata.methods (method_hash, projectID, startversiontime, file, "
									"startversionhash, endversiontime, endversionhash, name, lineNumber, authors, "
									"parserversion) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

	// Prepare query used to update a method in the database.
	updateMethods = prepareStatement("UPDATE projectdata.methods SET endVersionTime = ?, endVersionHash = ?, name = ?, "
									 "lineNumber = ?, authors = authors + ? WHERE method_hash = ? AND projectID = ? "
									 "AND file = ? AND startVersionTime = ?");

	// Prepare query used to select a method from the database given a hash, projectID and a file location.
	selectMethod =
		prepareStatement("SELECT * FROM projectdata.methods WHERE method_hash = ? AND projectID = ? AND file = ?");

	// Prepare query used to select the unchanged methods based on hash, projectID and file location.
	selectUnchangedMethods = prepareStatement(
		"SELECT method_hash, projectid, file, startversiontime, endversiontime, linenumber, name, startversionhash, "
		"endversionhash FROM projectdata.methods WHERE method_hash IN ? AND projectID = ? AND file IN ?");

	// Prepare query used to relate an author to a certain method (inside the method_by_author table).
	insertMethodByAuthor = prepareStatement("INSERT INTO projectdata.method_by_author (authorID, hash, "
											"startversiontime, file, projectID) VALUES (?, ?, ?, ?, ?)");

	// Prepare query used to select a method given the ID of an author (by means of the method_by_author table).
	selectMethodByAuthor = prepareStatement("SELECT * FROM projectdata.method_by_author WHERE authorid = ?");

	// Prepare query used to relate an author to an ID of the author (inside the author_by_id table).
	insertAuthorByID =
		prepareStatement("INSERT INTO projectdata.author_by_id (authorID, name, mail) VALUES (?, ?, ?) IF NOT EXISTS");

	// Prepare query used to select an author given its ID (bby means of the author_by_id table).
	selectAuthorByID = prepareStatement("SELECT * FROM projectdata.author_by_id WHERE authorid = ?");
}

const CassPrepared* DatabaseHandler::prepareStatement(std::string query)
{
	CassFuture *prepareFuture =	cass_session_prepare(connection, query.c_str());
	CassError rc = cass_future_error_code(prepareFuture);
	if (rc != 0)
	{
		// An error occurred which is handled below.
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
	CassStatement* query = cass_prepared_bind(selectMethods);

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

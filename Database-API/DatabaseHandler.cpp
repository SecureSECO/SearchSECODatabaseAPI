/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "DatabaseHandler.h"
#include <iostream>

using namespace std;

void DatabaseHandler::connect()
{
	CassFuture* connectFuture = NULL;
	CassCluster* cluster = cass_cluster_new();
	connection = cass_session_new();

	// Add contact points.
	cass_cluster_set_contact_points(cluster, "cassandra");
	cass_cluster_set_port(cluster, 8002);
	cass_cluster_set_protocol_version(cluster, CASS_PROTOCOL_VERSION_V3);

	// Provide the cluster object as configuration to connect the session.
	connectFuture = cass_session_connect(connection, cluster);

	CassError rc = cass_future_error_code(connectFuture);

	if (rc != CASS_OK)
	{
    	// Display connection error message.
    	const char* message;
    	size_t messageLength;
    	cass_future_error_message(connectFuture, &message, &messageLength);
    	fprintf(stderr, "Connect error: '%.*s'\n", (int)messageLength, message);
  	}
}

vector<MethodOut> DatabaseHandler::hashToMethods(string hash)
{
	CassStatement* query = cass_statement_new("SELECT * FROM projectdata.methods WHERE method_hash = ?", 1);

	cass_statement_bind_string(query, 0, hash.c_str());

	CassFuture* resultFuture = cass_session_execute(connection, query);

	vector<MethodOut> methods;

	if (cass_future_error_code(resultFuture) == CASS_OK)
	{
		const CassResult* result = cass_future_get_result(resultFuture);

		CassIterator* iterator = cass_iterator_from_result(result);

		// Add matches to result list.
		while(cass_iterator_next(iterator))
		{
			const CassRow* row = cass_iterator_get_row(iterator);
			methods.push_back(getMethod(row));
		}

		cass_iterator_free(iterator);

		cass_result_free(result);

	}
	else
	{
    	// Handle error.
    	const char* message;
    	size_t messageLength;
    	cass_future_error_message(resultFuture, &message, &messageLength);
    	fprintf(stderr, "Unable to run query: '%.*s'\n", (int)messageLength, message);
	}

	cass_statement_free(query);
	cass_future_free(resultFuture);

	return methods;
}

void DatabaseHandler::addProject(Project project)
{
	CassStatement* query = cass_statement_new("INSERT INTO projectdata.projects (projectID, version, license, name, url, ownerid) VALUES (?, ?, ?, ?, ?, ?)", 6);

	cass_statement_bind_int64(query, 0, project.projectID);

	cass_statement_bind_int64(query, 1, project.version);

	cass_statement_bind_string(query, 2, project.license.c_str());

	cass_statement_bind_string(query, 3, project.name.c_str());

	cass_statement_bind_string(query, 4, project.url.c_str());

	cass_statement_bind_uuid(query, 5, getAuthorID(project.owner));

	CassFuture* queryFuture = cass_session_execute(connection, query);

	// Statement objects can be freed immediately after being executed.
	cass_statement_free(query);

	// This will block until the query has finished.
	CassError rc = cass_future_error_code(queryFuture);

	if (rc != 0)
	{
		printf("Query result: %s\n", cass_error_desc(rc));
	}

	cass_future_free(queryFuture);
}

void DatabaseHandler::addMethod(MethodIn method, Project project)
{
	CassStatement* query = cass_statement_new("INSERT INTO projectdata.methods (method_hash, version, projectID, name, file, lineNumber ,authors) VALUES (?, ?, ?, ?, ?, ?, ?)", 7);

	cass_statement_bind_string(query, 0, method.hash.c_str());

	cass_statement_bind_int64(query, 1, project.version);

	cass_statement_bind_int64(query, 2, project.projectID);

	cass_statement_bind_string(query, 3, method.methodName.c_str());

	cass_statement_bind_string(query, 4, method.fileLocation.c_str());

	cass_statement_bind_int32(query, 5, method.lineNumber);

	int size = method.authors.size();

	CassCollection* authors = cass_collection_new(CASS_COLLECTION_TYPE_SET, size);

	for (int i = 0; i < size; i++)
	{
		cass_collection_append_uuid(authors, getAuthorID(method.authors[i]));
	}

	cass_statement_bind_collection(query, 6, authors);

	cass_collection_free(authors);

	CassFuture* queryFuture = cass_session_execute(connection, query);

	// Statement objects can be freed immediately after being executed.
	cass_statement_free(query);

	// This will block until the query has finished.
	CassError rc = cass_future_error_code(queryFuture);

	if (rc != 0)
	{
		printf("Query result: %s\n", cass_error_desc(rc));
	}

	cass_future_free(queryFuture);
}

void DatabaseHandler::addMethodByAuthor(CassUuid authorID, MethodIn method, Project project)
{
	CassStatement* query = cass_statement_new("INSERT INTO projectdata.method_by_author (authorID, hash, version, projectID) VALUES (?, ?, ?, ?)", 4);

	cass_statement_bind_uuid(query, 0, authorID);

	cass_statement_bind_string(query, 1, method.hash.c_str());

	cass_statement_bind_int64(query, 2, project.version);

	cass_statement_bind_int64(query, 3, project.projectID);

	CassFuture* queryFuture = cass_session_execute(connection, query);

	// Statement objects can be freed immediately after being executed.
	cass_statement_free(query);

	// This will block until the query has finished.
	CassError rc = cass_future_error_code(queryFuture);

	if (rc != 0)
	{
		printf("Query result: %s\n", cass_error_desc(rc));
	}

	cass_future_free(queryFuture);
}

CassUuid DatabaseHandler::getAuthorID(Author author)
{
	CassStatement* query = cass_statement_new("SELECT authorID FROM projectdata.id_by_author WHERE name = ? AND mail = ?", 2);
	cass_statement_bind_string(query, 0, author.name.c_str());
	cass_statement_bind_string(query, 1, author.mail.c_str());
	CassFuture* queryFuture = cass_session_execute(connection, query);

	CassUuid authorID;

	if (cass_future_error_code(queryFuture) == CASS_OK)
	{
		const CassResult* result = cass_future_get_result(queryFuture);

		if (cass_result_row_count(result) >= 1)
		{
			const CassRow* row = cass_result_first_row(result);
			const CassValue* id = cass_row_get_column(row, 0);
			cass_value_get_uuid(id, &authorID);
		}
		else
		{
			authorID = createAuthor(author);
		}

		cass_result_free(result);
	}
	else
	{
		// Handle error.
		const char* message;
		size_t messageLength;
		cass_future_error_message(queryFuture, &message, &messageLength);
		fprintf(stderr, "Unable to run query: '%.*s'\n", (int)messageLength, message);
	}

	cass_statement_free(query);
	cass_future_free(queryFuture);

	return authorID;
}

CassUuid DatabaseHandler::createAuthor(Author author)
{
	CassStatement* insertQuery = cass_statement_new("INSERT INTO projectdata.id_by_author (authorID, name, mail) VALUES (uuid(), ?, ?)", 2);
	cass_statement_bind_string(insertQuery, 0, author.name.c_str());
	cass_statement_bind_string(insertQuery, 1, author.mail.c_str());

	CassFuture* future = cass_session_execute(connection, insertQuery);

	cass_future_wait(future);

	cass_statement_free(insertQuery);

	cass_future_free(future);

	CassUuid authorID = getAuthorID(author);

	CassStatement* insertQuery2 = cass_statement_new("INSERT INTO projectdata.author_by_id (authorID, name, mail) VALUES (?, ?, ?)", 3);
	cass_statement_bind_uuid(insertQuery2, 0, authorID);
	cass_statement_bind_string(insertQuery2, 1, author.name.c_str());
	cass_statement_bind_string(insertQuery2, 2, author.mail.c_str());
	cass_session_execute(connection, insertQuery2);

	cass_statement_free(insertQuery2);

	return authorID;
}

MethodOut DatabaseHandler::getMethod(const CassRow* row)
{
	MethodOut method;

	method.hash = getString(row, "method_hash");
	method.methodName = getString(row, "name");
	method.fileLocation = getString(row, "file");

	method.lineNumber = getInt32(row, "lineNumber");
	method.projectID = getInt64(row, "projectID");
	method.version = getInt64(row, "version");

	const CassValue* set = cass_row_get_column(row, 3);
	CassIterator* iterator = cass_iterator_from_collection(set);

	if (iterator)
	{
		while (cass_iterator_next(iterator))
		{
			char authorId[CASS_UUID_STRING_LENGTH];
			CassUuid authorID;
			const CassValue* id = cass_iterator_get_value(iterator);
			cass_value_get_uuid(id, &authorID);
			cass_uuid_string(authorID, authorId);
			method.authorIDs.push_back(authorId);
		}
	}

	cass_iterator_free(iterator);

	return method;
}

string DatabaseHandler::getString(const CassRow* row, const char* column)
{
	const char* result;
	size_t len;
	const CassValue* value = cass_row_get_column_by_name(row, column);
	cass_value_get_string(value, &result, &len);
	return string(result, len);
}

int DatabaseHandler::getInt32(const CassRow* row, const char* column)
{
	cass_int32_t result;
	const CassValue* value = cass_row_get_column_by_name(row, column);
	cass_value_get_int32(value, &result);
	return result;
}

long long DatabaseHandler::getInt64(const CassRow* row, const char* column)
{
	cass_int64_t result;
	const CassValue* value = cass_row_get_column_by_name(row, column);
	cass_value_get_int64(value, &result);
	return result;
}

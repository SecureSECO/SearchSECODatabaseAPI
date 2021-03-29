#include "DatabaseHandler.h"
#include <iostream>

using namespace std;

void DatabaseHandler::Connect()
{
	CassFuture* connect_future = NULL;
	CassCluster* cluster = cass_cluster_new();
    connection = cass_session_new();

	/* Add contact points */
    cass_cluster_set_contact_points(cluster, "127.0.0.1");
    cass_cluster_set_protocol_version(cluster, CASS_PROTOCOL_VERSION_V3);

	/* Provide the cluster object as configuration to connect the session */
    connect_future = cass_session_connect(connection, cluster);

	CassError rc = cass_future_error_code(connect_future);

	if (rc != CASS_OK) {
    /* Display connection error message */
    const char* message;
    size_t message_length;
    cass_future_error_message(connect_future, &message, &message_length);
    fprintf(stderr, "Connect error: '%.*s'\n", (int)message_length, message);
  	}
}

vector<MethodOut> DatabaseHandler::HashToMethods(string hash)
{
	CassStatement* query = cass_statement_new("SELECT * FROM projectdata.methods WHERE method_hash = ?", 1);

	//CassUuid method_hash;
	//cass_uuid_from_string(hash.c_str(), &method_hash);
	//cass_statement_bind_uuid(query, 0, method_hash);

	cass_statement_bind_string(query, 0, hash.c_str());

	CassFuture* result_future = cass_session_execute(connection, query);

	vector<MethodOut> methods;

	if (cass_future_error_code(result_future) == CASS_OK) {
		const CassResult* result = cass_future_get_result(result_future);

		CassIterator* iterator = cass_iterator_from_result(result);

		while(cass_iterator_next(iterator)) {
			const CassRow* row = cass_iterator_get_row(iterator);
			methods.push_back(GetMethod(row));
		}

		cass_iterator_free(iterator);

		cass_result_free(result);

	} else {
      /* Handle error */
      const char* message;
      size_t message_length;
      cass_future_error_message(result_future, &message, &message_length);
      fprintf(stderr, "Unable to run query: '%.*s'\n", (int)message_length, message);
    }

	cass_statement_free(query);
	cass_future_free(result_future);

	return methods;
}

void DatabaseHandler::AddProject(Project project)
{
	CassStatement* query = cass_statement_new("INSERT INTO projectdata.projects (projectID, version, license, name, url, ownerid, stars, hashes) VALUES (?, ?, ?, ?, ?, ?, ?, ?)", 8);

	CassUuid projectID;
	cass_uuid_from_string(project.projectID.c_str(), &projectID);
	cass_statement_bind_uuid(query, 0, projectID);

	cass_statement_bind_int64(query, 1, project.version);

	cass_statement_bind_string(query, 2, project.license.c_str());

	cass_statement_bind_string(query, 3, project.name.c_str());

	cass_statement_bind_string(query, 4, project.url.c_str());

	cass_statement_bind_uuid(query, 5, GetAuthorID(project.owner));

	cass_statement_bind_int32(query, 6, project.stars);

	int size = project.hashes.size();

	CassCollection* hashes = cass_collection_new(CASS_COLLECTION_TYPE_SET, size);

	for(int i = 0; i < size; i++){
		//CassUuid hash;
		//cass_uuid_from_string(project.hashes[i].c_str(), &hash);
		//cass_collection_append_uuid(hashes, hash);
		cass_collection_append_string(hashes, project.hashes[i].c_str());
	}

	cass_statement_bind_collection(query, 7, hashes);

	cass_collection_free(hashes);

	CassFuture* query_future = cass_session_execute(connection, query);

	/* Statement objects can be freed immediately after being executed */
  cass_statement_free(query);

  /* This will block until the query has finished */
  CassError rc = cass_future_error_code(query_future);

  printf("Query result: %s\n", cass_error_desc(rc));

  cass_future_free(query_future);
}

void DatabaseHandler::AddMethod(MethodIn method, Project project)
{
	CassStatement* query = cass_statement_new("INSERT INTO projectdata.methods (method_hash, version, projectID, name, file, lineNumber ,authors) VALUES (?, ?, ?, ?, ?, ?, ?)", 7);

	//CassUuid hash;
	//cass_uuid_from_string(method.hash.c_str(), &hash);
	//cass_statement_bind_uuid(query, 0, hash);

	cass_statement_bind_string(query, 0, method.hash.c_str());

    cass_statement_bind_int64(query, 1, project.version);

	CassUuid projectID;
	cass_uuid_from_string(project.projectID.c_str(), &projectID);
 	cass_statement_bind_uuid(query, 2, projectID);

	cass_statement_bind_string(query, 3, method.methodName.c_str());

	cass_statement_bind_string(query, 4, method.fileLocation.c_str());

	cass_statement_bind_int32(query, 5, method.lineNumber);

	int size = method.authors.size();

	CassCollection* authors = cass_collection_new(CASS_COLLECTION_TYPE_SET, size);

	for(int i = 0; i < size; i++){
		cass_collection_append_uuid(authors, GetAuthorID(method.authors[i]));
	}

	cass_statement_bind_collection(query, 6, authors);

	cass_collection_free(authors);

	CassFuture* query_future = cass_session_execute(connection, query);

      /* Statement objects can be freed immediately after being executed */
    cass_statement_free(query);

    /* This will block until the query has finished */
    CassError rc = cass_future_error_code(query_future);

	if(rc != 0){
    	printf("Query result: %s\n", cass_error_desc(rc));
	}

    cass_future_free(query_future);
}

void DatabaseHandler::AddMethodByAuthor(CassUuid authorID, MethodIn method, Project project)
{
	CassStatement* query = cass_statement_new("INSERT INTO projectdata.method_by_author (authorID, hash, version, projectID) VALUES (?, ?, ?, ?)", 4);

	cass_statement_bind_uuid(query, 0, authorID);

	cass_statement_bind_string(query, 1, method.hash.c_str());

	cass_statement_bind_int64(query, 2, project.version);

	CassUuid projectID;
    cass_uuid_from_string(project.projectID.c_str(), &projectID);
    cass_statement_bind_uuid(query, 3, projectID);

	CassFuture* query_future = cass_session_execute(connection, query);

    /* Statement objects can be freed immediately after being executed */
    cass_statement_free(query);

    /* This will block until the query has finished */
    CassError rc = cass_future_error_code(query_future);

	if(rc != 0){
    	printf("Query result: %s\n", cass_error_desc(rc));
	}

    cass_future_free(query_future);
}

CassUuid DatabaseHandler::GetAuthorID(Author author)
{
	CassStatement* query = cass_statement_new("SELECT authorID FROM projectdata.id_by_author WHERE name = ? AND mail = ?", 2);

	cass_statement_bind_string(query, 0, author.name.c_str());

	cass_statement_bind_string(query, 1, author.mail.c_str());

	CassFuture* query_future = cass_session_execute(connection, query);

	CassUuid authorID;

	if (cass_future_error_code(query_future) == CASS_OK) {
		const CassResult* result = cass_future_get_result(query_future);

		cout << cass_result_row_count(result) << endl;

		if (cass_result_row_count(result) >= 1) {

		const CassRow* row = cass_result_first_row(result);

		const CassValue* id = cass_row_get_column(row, 0);

		cass_value_get_uuid(id, &authorID);

		} else {
			CassStatement* insertQuery = cass_statement_new("INSERT INTO projectdata.id_by_author (authorID, name, mail) VALUES (uuid(), ?, ?)", 2);
			cass_statement_bind_string(insertQuery, 0, author.name.c_str());
			cass_statement_bind_string(insertQuery, 1, author.mail.c_str());

			CassFuture* future = cass_session_execute(connection, insertQuery);

			cass_future_wait(future);

			cass_statement_free(insertQuery);

			cass_future_free(future);

			cout << "Getting id" << endl;

			authorID = GetAuthorID(author);

			CassStatement* insertQuery2 = cass_statement_new("INSERT INTO projectdata.author_by_id (authorID, name, mail) VALUES (?, ?, ?)", 3);
			cass_statement_bind_uuid(insertQuery2, 0, authorID);
			cass_statement_bind_string(insertQuery2, 1, author.name.c_str());
			cass_statement_bind_string(insertQuery2, 2, author.mail.c_str());
			cass_session_execute(connection, insertQuery2);

			cass_statement_free(insertQuery2);
		}

		cass_result_free(result);

	} else {
      /* Handle error */
      const char* message;
      size_t message_length;
      cass_future_error_message(query_future, &message, &message_length);
      fprintf(stderr, "Unable to run query: '%.*s'\n", (int)message_length, message);
    }

	cass_statement_free(query);
	cass_future_free(query_future);

	return authorID;
}

MethodOut DatabaseHandler::GetMethod(const CassRow* row)
{
	MethodOut method;

	/*char method_hash[CASS_UUID_STRING_LENGTH];
	CassUuid hash_uuid;
	const CassValue* hash = cass_row_get_column(row, 0);
	cass_value_get_uuid(hash, &hash_uuid);
	cass_uuid_string(hash_uuid, method_hash);
	method.hash = method_hash;*/

	const char* method_hash;
	size_t len;
	const CassValue* hash = cass_row_get_column(row,0);
	cass_value_get_string(hash, &method_hash, &len);
	method.hash = string(method_hash, len);

	const char* method_name;
	const CassValue* name = cass_row_get_column(row, 7);
	cass_value_get_string(name, &method_name, &len);
	method.methodName = string(method_name, len);

	const char* method_file;
    //size_t len;
    const CassValue* file = cass_row_get_column(row, 5);
    cass_value_get_string(file, &method_file, &len);
    method.fileLocation = string(method_file, len);

	cass_int32_t lineNumber;
	const CassValue* number = cass_row_get_column(row, 6);
	cass_value_get_int32(number, &lineNumber);
	method.lineNumber = lineNumber;

	char project_id[CASS_UUID_STRING_LENGTH];
    CassUuid id_uuid;
    const CassValue* projectID = cass_row_get_column(row, 2);
    cass_value_get_uuid(projectID, &id_uuid);
    cass_uuid_string(id_uuid, project_id);
    method.projectID = project_id;

	time_t project_version;
	cass_int64_t version;
	const CassValue* projectVersion = cass_row_get_column(row, 1);
	cass_value_get_int64(projectVersion, &version);
	method.version = version;

	const CassValue* set = cass_row_get_column(row, 3);
	CassIterator* iterator = cass_iterator_from_collection(set);
	while(cass_iterator_next(iterator)){
		char author_id[CASS_UUID_STRING_LENGTH];
		CassUuid authorID;
		const CassValue* id = cass_iterator_get_value(iterator);
		cass_value_get_uuid(id, &authorID);
		cass_uuid_string(authorID, author_id);
		method.authorIDs.push_back(author_id);
	}

	cass_iterator_free(iterator);

	return method;
}

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

vector<Method> DatabaseHandler::HashToMethods(string hash)
{
	CassStatement* query = cass_statement_new("SELECT * FROM projectdata.methods WHERE method_hash = ?", 1);

	CassUuid method_hash;
	cass_uuid_from_string(hash.c_str(), &method_hash);
	cass_statement_bind_uuid(query, 0, method_hash);

	CassFuture* result_future = cass_session_execute(connection, query);

	vector<Method> methods;

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
	CassStatement* query = cass_statement_new("INSERT INTO projectdata.projects (projectID, version, license, name, url, owner, stars, hashes) VALUES (?, ?, ?, ?, ?, ?, ?, ?)", 8);

	CassUuid projectID;
	cass_uuid_from_string(project.projectID.c_str(), &projectID);
	cass_statement_bind_uuid(query, 0, projectID);

	cass_statement_bind_int64(query, 1, project.version);

	cass_statement_bind_string(query, 2, project.license.c_str());

	cass_statement_bind_string(query, 3, project.name.c_str());

	cass_statement_bind_string(query, 4, project.url.c_str());

	/* Get schema object (this should be cached) */
	const CassSchemaMeta* schema_meta = cass_session_get_schema_meta(connection);

  /* Get the keyspace for the user-defined type. It doesn't need to be freed */
  const CassKeyspaceMeta* keyspace_meta =
    cass_schema_meta_keyspace_by_name(schema_meta, "projectdata");

  /* This data type object doesn't need to be freed */
  const CassDataType* author_data_type =
    cass_keyspace_meta_user_type_by_name(keyspace_meta, "author");

  /* ... */

  /* Schema object must be freed */
  cass_schema_meta_free(schema_meta);

	CassUserType* owner = cass_user_type_new_from_data_type(author_data_type);
	cass_user_type_set_string(owner, 0, project.owner.name.c_str());
	cass_user_type_set_string(owner, 1, project.owner.mail.c_str());
	cass_statement_bind_user_type(query, 5, owner);

	cass_statement_bind_int32(query, 6, project.stars);

	int size = project.hashes.size();

	CassCollection* hashes = cass_collection_new(CASS_COLLECTION_TYPE_SET, size);

	for(int i = 0; i < size; i++){
		CassUuid hash;
		cass_uuid_from_string(project.hashes[i].c_str(), &hash);
		cass_collection_append_uuid(hashes, hash);
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

void DatabaseHandler::AddMethod(Method method, Project project)
{
	CassStatement* query = cass_statement_new("INSERT INTO projectdata.methods (method_hash, version, projectID, name, file, authors) VALUES (?, ?, ?, ?, ?, ?)", 6);

	CassUuid hash;
	cass_uuid_from_string(method.hash.c_str(), &hash);
	cass_statement_bind_uuid(query, 0, hash);

    cass_statement_bind_int64(query, 1, project.version);

	CassUuid projectID;
	cass_uuid_from_string(project.projectID.c_str(), &projectID);
 	cass_statement_bind_uuid(query, 2, projectID);

	cass_statement_bind_string(query, 3, method.methodName.c_str());

	cass_statement_bind_string(query, 4, method.fileLocation.c_str());

	/* Get schema object (this should be cached) */
    const CassSchemaMeta* schema_meta = cass_session_get_schema_meta(connection);

    /* Get the keyspace for the user-defined type. It doesn't need to be freed */
    const CassKeyspaceMeta* keyspace_meta =
     cass_schema_meta_keyspace_by_name(schema_meta, "projectdata");

    /* This data type object doesn't need to be freed */
    const CassDataType* author_data_type =
    cass_keyspace_meta_user_type_by_name(keyspace_meta, "author");

    /* ... */

    /* Schema object must be freed */
	cass_schema_meta_free(schema_meta);

	int size = method.authors.size();

	CassCollection* authors = cass_collection_new(CASS_COLLECTION_TYPE_SET, size);

	for(int i = 0; i < size; i++){
		CassUserType* author = cass_user_type_new_from_data_type(author_data_type);
	    cass_user_type_set_string(author, 0, method.authors[i].name.c_str());
	    cass_user_type_set_string(author, 1, method.authors[i].mail.c_str());
		cass_collection_append_user_type(authors, author);
	}

	cass_statement_bind_collection(query, 5, authors);

	cass_collection_free(authors);

	CassFuture* query_future = cass_session_execute(connection, query);

      /* Statement objects can be freed immediately after being executed */
    cass_statement_free(query);

    /* This will block until the query has finished */
    CassError rc = cass_future_error_code(query_future);

    printf("Query result: %s\n", cass_error_desc(rc));

    cass_future_free(query_future);
}

Method DatabaseHandler::GetMethod(const CassRow* row)
{
	Method method;

	char method_hash[CASS_UUID_STRING_LENGTH];
	CassUuid hash_uuid;
	const CassValue* hash = cass_row_get_column(row, 0);
	cass_value_get_uuid(hash, &hash_uuid);
	cass_uuid_string(hash_uuid, method_hash);
	method.hash = method_hash;

	return method;
}

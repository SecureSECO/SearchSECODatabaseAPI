/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "DatabaseConnection.h"

using namespace std;

#define MIN_AMOUNT_JOBS 5

void DatabaseConnection::connect(string ip, int port)
{
	CassFuture* connectFuture = NULL;
	CassCluster* cluster = cass_cluster_new();
	connection = cass_session_new();

	// Add contact points.
	cass_cluster_set_contact_points(cluster, ip.c_str());
	cass_cluster_set_port(cluster, port);
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

string DatabaseConnection::getJob()
{
	int jobAmount = getNumberOfJobs();
	// Check if number of jobs is enough to provide the top job.
	if (jobAmount >= MIN_AMOUNT_JOBS)
	{
		return getTopJob();
	}
	else
	{
		return "Crawl";
	}
}

string DatabaseConnection::getTopJob()
{
	CassStatement* query = cass_statement_new("SELECT * FROM jobs LIMIT 1", 0);
        cass_statement_set_consistency(query, CASS_CONSISTENCY_QUORUM);
        CassFuture* resultFuture = cass_session_execute(connection, query);
	 if (cass_future_error_code(resultFuture) == CASS_OK)
        {
		const CassResult* result = cass_future_get_result(resultFuture);
                const CassRow* row = cass_result_first_row(result);
		const char* url;
		size_t len;
		CassUuid id;
		cass_value_get_string(cass_row_get_column_by_name(row, "url"), &url, &len);
		cass_value_get_uuid(cass_row_get_column(row, 0), &id);
                cass_statement_free(query);
                cass_future_free(resultFuture);
		deleteTopJob(id);
                return url;
	}
	else
        {
                // Handle error.
                const char* message;
                size_t messageLength;
                cass_future_error_message(resultFuture, &message, &messageLength);
                fprintf(stderr, "Unable to get job: '%.*s'\n", (int)messageLength, message);
		cass_statement_free(query);
                return "";
        }
}

void DatabaseConnection::deleteTopJob(CassUuid id)
{
	CassStatement* query = cass_statement_new("DELETE FROM jobs WHERE job_id = ?", 1);

	cass_statement_set_consistency(query, CASS_CONSISTENCY_QUORUM);

	cass_statement_bind_uuid(query, 0, id);

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

int DatabaseConnection::getNumberOfJobs()
{
	CassStatement* query = cass_statement_new("SELECT COUNT(*) FROM jobs", 0);
	cass_statement_set_consistency(query, CASS_CONSISTENCY_QUORUM);
	CassFuture* resultFuture = cass_session_execute(connection, query);
	if (cass_future_error_code(resultFuture) == CASS_OK)
	{
		const CassResult* result = cass_future_get_result(resultFuture);
		const CassRow* row = cass_result_first_row(result);
		cass_int32_t count;
		cass_value_get_int32(cass_row_get_column_by_name(row, "count"), &count);
		cass_statement_free(query);
		cass_future_free(resultFuture);
		return count;
	}
	else
	{
		// Handle error.
		const char* message;
		size_t messageLength;
		cass_future_error_message(resultFuture, &message, &messageLength);
		fprintf(stderr, "Unable to get number of jobs: '%.*s'\n", (int)messageLength, message);
		cass_statement_free(query);
		return -1;
	}

}

void DatabaseConnection::uploadJob(string url)
{
	CassStatement* query = cass_statement_new("INSERT INTO jobs (jobID, task, url) VALUES (uuid(), \"spider\", ?)", 1);

        cass_statement_set_consistency(query, CASS_CONSISTENCY_QUORUM);

        cass_statement_bind_string(query, 0, url.c_str());

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

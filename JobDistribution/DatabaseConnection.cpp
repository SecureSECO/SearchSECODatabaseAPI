/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "DatabaseConnection.h"
#include <iostream>

using namespace std;

int numberOfJobs;
int crawlId;
bool alreadyCrawling = false;

void DatabaseConnection::connect(string ip, int port)
{
	CassFuture* connectFuture = NULL;
	CassCluster* cluster = cass_cluster_new();
	connection = cass_session_new();

	// Add contact points.
	cass_cluster_set_contact_points(cluster, ip.c_str());
	cass_cluster_set_port(cluster, port);
	cass_cluster_set_protocol_version(cluster, CASS_PROTOCOL_VERSION_V3);
	cass_cluster_set_consistency(cluster, CASS_CONSISTENCY_QUORUM);
	cass_cluster_set_num_threads_io(cluster, MAX_THREADS);

	// Provide the cluster object as configuration to connect the session.
	connectFuture = cass_session_connect_keyspace(connection, cluster, "jobs");

	CassError rc = cass_future_error_code(connectFuture);

	if (rc != CASS_OK)
	{
		// Display connection error message.
		const char* message;
		size_t messageLength;
		cass_future_error_message(connectFuture, &message, &messageLength);
		fprintf(stderr, "Connect error: '%.*s'\n", (int)messageLength, message);
		return;
  	}
	setPreparedStatements();
	// Set initial number of jobs in the queue.
	numberOfJobs = getNumberOfJobs();
	cout << to_string(numberOfJobs) + "\n";
	::crawlId = 0;
}

void DatabaseConnection::setPreparedStatements()
{
	CassFuture *prepareFuture = cass_session_prepare(connection, "SELECT * FROM jobs.jobsqueue LIMIT 1");
	CassError rc = cass_future_error_code(prepareFuture);
	preparedGetTopJob = cass_future_get_prepared(prepareFuture);

	prepareFuture = cass_session_prepare(connection, "DELETE FROM jobs.jobsqueue WHERE constant = 1 AND jobid = ?");
	rc = cass_future_error_code(prepareFuture);
	preparedDeleteTopJob = cass_future_get_prepared(prepareFuture);

	prepareFuture = cass_session_prepare(connection, "INSERT INTO jobs.jobsqueue (jobid, priority, url, constant) VALUES (uuid(), ?, ?, 1)");
        rc = cass_future_error_code(prepareFuture);
        preparedUploadJob = cass_future_get_prepared(prepareFuture);

	prepareFuture = cass_session_prepare(connection, "SELECT COUNT(*) FROM jobs.jobsqueue");
        rc = cass_future_error_code(prepareFuture);
        preparedAmountOfJobs = cass_future_get_prepared(prepareFuture);

	cass_future_free(prepareFuture);
}


string DatabaseConnection::getJob()
{
	// Check if number of jobs is enough to provide the top job.
	if (numberOfJobs >= MIN_AMOUNT_JOBS || (alreadyCrawling == true && numberOfJobs >= 1))
	{
		return "Spider?" +  getTopJob();
	}
	// If number of jobs is not high enough, the job is to crawl for more jobs.
	else if (alreadyCrawling == false)
	{
		alreadyCrawling = true;
		return "Crawl?" + to_string(crawlId);
	}
	else
	{
		return "NoJob";
	}
}

string DatabaseConnection::getTopJob()
{
	CassStatement* query = cass_prepared_bind(preparedGetTopJob);
        CassFuture* resultFuture = cass_session_execute(connection, query);
	 if (cass_future_error_code(resultFuture) == CASS_OK)
        {
		const CassResult* result = cass_future_get_result(resultFuture);
                const CassRow* row = cass_result_first_row(result);
		const char* url;
		size_t len;
		CassUuid id;
		cass_value_get_string(cass_row_get_column_by_name(row, "url"), &url, &len);
		cass_value_get_uuid(cass_row_get_column_by_name(row, "jobid"), &id);
                cass_statement_free(query);
                cass_future_free(resultFuture);
		//Delete the job that is returned.
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
	CassStatement* query = cass_prepared_bind(preparedDeleteTopJob);

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
	::numberOfJobs -= 1;

}

int DatabaseConnection::getNumberOfJobs()
{
	CassStatement* query = cass_prepared_bind(preparedAmountOfJobs);
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

void DatabaseConnection::uploadJob(string url, int priority)
{
	CassStatement* query = cass_prepared_bind(preparedUploadJob);

        cass_statement_bind_int32(query, 0, priority);

	cass_statement_bind_string(query, 1, url.c_str());

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
	::numberOfJobs += 1;
}

void DatabaseConnection::updateCrawlId(int id)
{
	::crawlId = id;
	alreadyCrawling = false;
}

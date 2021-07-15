/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "DatabaseConnection.h"
#include "HTTPStatus.h"
#include "Utility.h"

#include <iostream>
#include <chrono>
#include <unistd.h>

void DatabaseConnection::connect(std::string ip, int port)
{
	errno = 0;
	CassFuture* connectFuture = NULL;
	CassCluster* cluster = cass_cluster_new();
	connection = cass_session_new();

	// Add contact points.
	cass_cluster_set_contact_points(cluster, ip.c_str());
	cass_cluster_set_port(cluster, port);
	cass_cluster_set_protocol_version(cluster, CASS_PROTOCOL_VERSION_V3);
	cass_cluster_set_consistency(cluster, CASS_CONSISTENCY_QUORUM);
	cass_cluster_set_num_threads_io(cluster, MAX_THREADS);

	std::cout << "Connecting to the database." << std::endl;

	// Provide the cluster object as configuration to connect the session.
	connectFuture = cass_session_connect_keyspace(connection, cluster, "jobs");

	CassError rc = cass_future_error_code(connectFuture);

	if (rc != CASS_OK)
	{
		std::cout << "Could not connect to the database." << std::endl;
		std::cout << "Retrying in 45 seconds.." << std::endl;
		usleep(45000000);
		std::cout << "Retrying now." << std::endl;

		connectFuture = cass_session_connect_keyspace(connection, cluster, "projectdata");

		CassError rc = cass_future_error_code(connectFuture);

		if (rc != CASS_OK)
		{
			// A connection error occurred, which is handled below.
			const char *message;
			size_t messageLength;
			cass_future_error_message(connectFuture, &message, &messageLength);
			fprintf(stderr, "Connect error: '%.*s'\n", (int)messageLength, message);
			errno = ENETUNREACH;
			return;
		}
	}
	setPreparedStatements();
}

void DatabaseConnection::setPreparedStatements()
{
	CassFuture *prepareFuture =
		cass_session_prepare(connection, "SELECT * FROM jobs.jobsqueue WHERE constant = 1 LIMIT 1");
	CassError rc = cass_future_error_code(prepareFuture);
	preparedGetTopJob = cass_future_get_prepared(prepareFuture);

	prepareFuture = cass_session_prepare(
		connection, "DELETE FROM jobs.jobsqueue WHERE constant = 1 AND priority = ? AND jobid = ?");
	rc = cass_future_error_code(prepareFuture);
	preparedDeleteTopJob = cass_future_get_prepared(prepareFuture);

	prepareFuture = cass_session_prepare(
		connection, "INSERT INTO jobs.jobsqueue (jobid, priority, url, constant) VALUES (uuid(), ?, ?, 1)");
	rc = cass_future_error_code(prepareFuture);
	preparedUploadJob = cass_future_get_prepared(prepareFuture);

	prepareFuture = cass_session_prepare(connection, "SELECT COUNT(*) FROM jobs.jobsqueue WHERE constant = 1");
	rc = cass_future_error_code(prepareFuture);
	preparedAmountOfJobs = cass_future_get_prepared(prepareFuture);

	prepareFuture = cass_session_prepare(connection, "SELECT * FROM jobs.variables WHERE name = 'crawlID'");
	rc = cass_future_error_code(prepareFuture);
	preparedCrawlID = cass_future_get_prepared(prepareFuture);

	prepareFuture = cass_session_prepare(connection, "UPDATE jobs.variables SET value = ? WHERE name = 'crawlID'");
	rc = cass_future_error_code(prepareFuture);
	preparedUpdateCrawlID = cass_future_get_prepared(prepareFuture);

	cass_future_free(prepareFuture);
}

std::string DatabaseConnection::getTopJob()
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(preparedGetTopJob);
	CassFuture *resultFuture = cass_session_execute(connection, query);
	if (cass_future_error_code(resultFuture) == CASS_OK)
	{
		// Retrieve the result.
		const CassResult *result = cass_future_get_result(resultFuture);
		const CassRow *row = cass_result_first_row(result);
		const char *url;
		size_t len;
		CassUuid id;
		cass_int64_t priority;
		cass_value_get_string(cass_row_get_column_by_name(row, "url"), &url, &len);
		cass_value_get_uuid(cass_row_get_column_by_name(row, "jobid"), &id);
		cass_value_get_int64(cass_row_get_column_by_name(row, "priority"), &priority);
		cass_statement_free(query);
		cass_future_free(resultFuture);

		// Delete the job that is returned.
		deleteTopJob(id, priority);
		if (errno != 0)
		{
			return "";
		}
		std::string resultJob(url, len);
		return resultJob;
	}
	else
	{
		// An error occurred, which is handled below.
		const char *message;
		size_t messageLength;
		cass_future_error_message(resultFuture, &message, &messageLength);
		fprintf(stderr, "Unable to get job: '%.*s'\n", (int)messageLength, message);
		cass_statement_free(query);
		errno = ENETUNREACH;
		return "";
	}
}

void DatabaseConnection::deleteTopJob(CassUuid id, cass_int64_t priority)
{
	errno = 0;
	CassStatement* query = cass_prepared_bind(preparedDeleteTopJob);

	cass_statement_bind_int64(query, 0, priority);
	cass_statement_bind_uuid(query, 1, id);

	CassFuture* queryFuture = cass_session_execute(connection, query);

	// Statement objects can be freed immediately after being executed.
	cass_statement_free(query);

	// This will block until the query has finished.
	CassError rc = cass_future_error_code(queryFuture);

	if (rc != 0)
	{
		// An error occurred, which is handled below.
		printf("Query result: %s\n", cass_error_desc(rc));
		errno = ENETUNREACH;
	}
	cass_future_free(queryFuture);
}

int DatabaseConnection::getNumberOfJobs()
{
	errno = 0;
	CassStatement* query = cass_prepared_bind(preparedAmountOfJobs);
	CassFuture* resultFuture = cass_session_execute(connection, query);
	if (cass_future_error_code(resultFuture) == CASS_OK)
	{
		// Retrieve the result.
		const CassResult* result = cass_future_get_result(resultFuture);
		const CassRow* row = cass_result_first_row(result);
		cass_int64_t count;
		cass_value_get_int64(cass_row_get_column(row, 0), &count);
		cass_statement_free(query);
		cass_future_free(resultFuture);
		return count;
	}
	else
	{
		// An error occurred, which is handled below.
		const char* message;
		size_t messageLength;
		cass_future_error_message(resultFuture, &message, &messageLength);
		fprintf(stderr, "Unable to get number of jobs: '%.*s'\n", (int)messageLength, message);
		cass_statement_free(query);
		cass_future_free(resultFuture);
		errno = ENETUNREACH;
		return -1;
	}
}

int DatabaseConnection::getCrawlID()
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(preparedCrawlID);
	CassFuture *resultFuture = cass_session_execute(connection, query);
	if (cass_future_error_code(resultFuture) == CASS_OK)
	{
		// Retrieve the result.
		const CassResult *result = cass_future_get_result(resultFuture);
		const CassRow *row = cass_result_first_row(result);
		cass_int32_t id;
		cass_value_get_int32(cass_row_get_column(row, 1), &id);
		cass_statement_free(query);
		cass_future_free(resultFuture);
		return id;
	}
	else
	{
		// An error occurred, which is handled below.
		const char *message;
		size_t messageLength;
		cass_future_error_message(resultFuture, &message, &messageLength);
		fprintf(stderr, "Unable to get crawl ID: '%.*s'\n", (int)messageLength, message);
		cass_statement_free(query);
		cass_future_free(resultFuture);
		errno = ENETUNREACH;
		return 0;
	}
}

void DatabaseConnection::setCrawlID(int id)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(preparedUpdateCrawlID);
	cass_statement_bind_int32_by_name(query, "value", id);

	CassFuture *resultFuture = cass_session_execute(connection, query);

	if (cass_future_error_code(resultFuture) != CASS_OK)
	{
		// An error occurred, which is handled below.
		const char *message;
		size_t messageLength;
		cass_future_error_message(resultFuture, &message, &messageLength);
		fprintf(stderr, "Unable to get crawl ID: '%.*s'\n", (int)messageLength, message);
		cass_statement_free(query);
		cass_future_free(resultFuture);
		errno = ENETUNREACH;
	}
}

void DatabaseConnection::uploadJob(std::string url, long long priority)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(preparedUploadJob);

	auto currentTime = Utility::getCurrentTimeMilliSeconds();
	long long resultPriority = currentTime - priority;
	cass_statement_bind_int64(query, 0, resultPriority);
	cass_statement_bind_string(query, 1, url.c_str());

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

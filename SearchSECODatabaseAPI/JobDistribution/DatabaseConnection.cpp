/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "DatabaseConnection.h"
#include "HTTPStatus.h"
#include "Utility.h"
#include "DatabaseUtility.h"

#include <iostream>
#include <chrono>
#include <unistd.h>

void DatabaseConnection::connect(std::string ip, int port)
{
	connection = DatabaseUtility::connect(ip, port, "jobs");
	setPreparedStatements();
}

void DatabaseConnection::setPreparedStatements()
{
	preparedGetTopJob =
		DatabaseUtility::prepareStatement(connection, "SELECT * FROM jobs.jobsqueue WHERE constant = 1 LIMIT 1");

	preparedDeleteTopJob = DatabaseUtility::prepareStatement(
		connection, "DELETE FROM jobs.jobsqueue WHERE constant = 1 AND priority = ? AND jobid = ?");

	preparedAddCurrentJob = DatabaseUtility::prepareStatement(
		connection,
		"INSERT INTO jobs.currentjobs (jobid, time, timeout, priority, url, retries) VALUES (?, ?, ?, ?, ?, ?)");

	preparedGetCurrentJob =
		DatabaseUtility::prepareStatement(connection, "SELECT * FROM jobs.currentjobs WHERE jobid = ?");

	preparedDeleteCurrentJob = DatabaseUtility::prepareStatement(
		connection, "DELETE FROM jobs.currentjobs WHERE jobid = ?");

	preparedAddFailedJob =
		DatabaseUtility::prepareStatement(connection, "INSERT INTO jobs.failedjobs (jobid, time, timeout, priority, url, "
													  "retries, reasonID, reasonData) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");

	preparedUploadJob = DatabaseUtility::prepareStatement(
		connection,
		"INSERT INTO jobs.jobsqueue (jobid, priority, url, constant, retries, timeout) VALUES (uuid(), ?, ?, 1, ?, ?)");

	preparedAmountOfJobs =
		DatabaseUtility::prepareStatement(connection, "SELECT COUNT(*) FROM jobs.jobsqueue WHERE constant = 1");

	preparedCrawlID =
		DatabaseUtility::prepareStatement(connection, "SELECT * FROM jobs.variables WHERE name = 'crawlID'");

	preparedUpdateCrawlID =
		DatabaseUtility::prepareStatement(connection, "UPDATE jobs.variables SET value = ? WHERE name = 'crawlID'");
}

Job DatabaseConnection::getTopJob()
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(preparedGetTopJob);
	CassFuture *resultFuture = cass_session_execute(connection, query);
	Job job;
	if (cass_future_error_code(resultFuture) == CASS_OK)
	{
		// Retrieve the result.
		const CassResult *result = cass_future_get_result(resultFuture);
		const CassRow *row = cass_result_first_row(result);
		const char *url;
		size_t len;
		CassUuid id;
		cass_int64_t priority;
		cass_int32_t retries;
		cass_int64_t timeout;

		cass_value_get_string(cass_row_get_column_by_name(row, "url"), &url, &len);
		cass_value_get_uuid(cass_row_get_column_by_name(row, "jobid"), &id);
		cass_value_get_int64(cass_row_get_column_by_name(row, "priority"), &priority);
		cass_value_get_int32(cass_row_get_column_by_name(row, "retries"), &retries);
		cass_value_get_int64(cass_row_get_column_by_name(row, "timeout"), &timeout);
		cass_statement_free(query);
		cass_future_free(resultFuture);
		
		std::string resultUrl(url, len);

		// Add job to list of current jobs.
		long long currTime = addCurrentJob(id, timeout, priority, resultUrl, retries);
		if (errno != 0)
		{
			return job;
		}

		// Delete the job that is returned.
		deleteTopJob(id, priority);
		if (errno != 0)
		{
			// Do not delete current job, to prevent a newer version of the job from being accidentally deleted.
			return job;
		}

		char jobid[CASS_UUID_STRING_LENGTH];
		cass_uuid_string(id, jobid);

		// Return result.
		
		job.priority = priority;
		job.jobid = jobid;
		job.url = url;
		job.retries = retries;
		job.time = currTime;
		job.timeout = timeout;
		return job;
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
		return job;
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

long long DatabaseConnection::addCurrentJob(Job job)
{
	CassUuid jobid;
	cass_uuid_from_string(job.jobid.c_str(), &jobid);
	return addCurrentJob(jobid, job.timeout, job.priority, job.url, job.retries);
}

long long DatabaseConnection::addCurrentJob(CassUuid id, long long timeout, long long priority, std::string url, int retries)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(preparedAddCurrentJob);

	cass_statement_bind_uuid_by_name(query, "jobid", id);
	long long currTime = Utility::getCurrentTimeMilliSeconds();
	cass_statement_bind_int64_by_name(query, "time", currTime);
	cass_statement_bind_int64_by_name(query, "timeout", timeout);
	cass_statement_bind_int64_by_name(query, "priority", priority);
	cass_statement_bind_string_by_name(query, "url", url.c_str());
	cass_statement_bind_int32_by_name(query, "retries", retries);	

	CassFuture *queryFuture = cass_session_execute(connection, query);

	// Statement objects can be freed immediately after being executed.
	cass_statement_free(query);

	// This will block until the query has finished.
	CassError rc = cass_future_error_code(queryFuture);

	if (rc != 0)
	{
		printf("Unable to add current job: %s\n", cass_error_desc(rc));
		errno = ENETUNREACH;
	}
	cass_future_free(queryFuture);
	return currTime;
}

Job DatabaseConnection::getCurrentJob(std::string jobid)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(preparedGetCurrentJob);
	CassUuid id;
	cass_uuid_from_string(jobid.c_str(), &id);

	cass_statement_bind_uuid_by_name(query, "jobid", id);

	CassFuture *resultFuture = cass_session_execute(connection, query);

	Job job;
	job.jobid = "";

	if (cass_future_error_code(resultFuture) == CASS_OK)
	{
		// Retrieve the result.
		const CassResult *result = cass_future_get_result(resultFuture);
		if (cass_result_row_count(result) >= 1)
		{
			const CassRow *row = cass_result_first_row(result);
			job = retrieveCurrentJob(row);
			cass_statement_free(query);
			cass_future_free(resultFuture);
			deleteCurrentJob(id);
		}
	}
	else
	{
		// An error occurred, which is handled below.
		const char *message;
		size_t messageLength;
		cass_future_error_message(resultFuture, &message, &messageLength);
		fprintf(stderr, "Unable to get number of jobs: '%.*s'\n", (int)messageLength, message);
		cass_statement_free(query);
		cass_future_free(resultFuture);
		errno = ENETUNREACH;
	}
	return job;
}

void DatabaseConnection::deleteCurrentJob(CassUuid id)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(preparedDeleteCurrentJob);

	cass_statement_bind_uuid_by_name(query, "jobid", id);

	CassFuture *queryFuture = cass_session_execute(connection, query);

	// Statement objects can be freed immediately after being executed.
	cass_statement_free(query);

	// This will block until the query has finished.
	CassError rc = cass_future_error_code(queryFuture);

	if (rc != 0)
	{
		// An error occurred, which is handled below.
		printf("Could not delete current job: %s\n", cass_error_desc(rc));
		errno = ENETUNREACH;
	}
	cass_future_free(queryFuture);
}

void DatabaseConnection::addFailedJob(std::string id, long long currTime, long long timeout, long long priority,
									   std::string url, int retries, int reasonID, std::string reasonData)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(preparedAddFailedJob);

	CassUuid jobid;
	cass_uuid_from_string(id.c_str(), &jobid);

	cass_statement_bind_uuid_by_name(query, "jobid", jobid);
	cass_statement_bind_int64_by_name(query, "time", currTime);
	cass_statement_bind_int64_by_name(query, "timeout", timeout);
	cass_statement_bind_int64_by_name(query, "priority", priority);
	cass_statement_bind_string_by_name(query, "url", url.c_str());
	cass_statement_bind_int32_by_name(query, "retries", retries);
	cass_statement_bind_int32_by_name(query, "reasonID", reasonID);
	cass_statement_bind_string_by_name(query, "reasonData", reasonData.c_str());

	CassFuture *queryFuture = cass_session_execute(connection, query);

	// Statement objects can be freed immediately after being executed.
	cass_statement_free(query);

	// This will block until the query has finished.
	CassError rc = cass_future_error_code(queryFuture);

	if (rc != 0)
	{
		printf("Unable to add failed job: %s\n", cass_error_desc(rc));
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
		int id = DatabaseUtility::getInt32(row, "value");
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

void DatabaseConnection::uploadJob(std::string url, long long priority, int retries, long long timeout)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(preparedUploadJob);

	auto currentTime = Utility::getCurrentTimeMilliSeconds();
	long long resultPriority = currentTime - priority;
	cass_statement_bind_int64_by_name(query, "priority", resultPriority);
	cass_statement_bind_string_by_name(query, "url", url.c_str());
	cass_statement_bind_int32_by_name(query, "retries", retries);
	cass_statement_bind_int64_by_name(query, "timeout", timeout);

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

Job DatabaseConnection::retrieveCurrentJob(const CassRow *row)
{
	Job job;

	// Retrieve the values of the variables in the row.
	job.jobid = DatabaseUtility::getUUID(row, "jobid");
	job.time = DatabaseUtility::getInt64(row, "time");
	job.timeout = DatabaseUtility::getInt64(row, "timeout");
	job.priority = DatabaseUtility::getInt64(row, "priority");
	job.url = DatabaseUtility::getString(row, "url");
	job.retries = DatabaseUtility::getInt32(row, "retries");

	return job;
}

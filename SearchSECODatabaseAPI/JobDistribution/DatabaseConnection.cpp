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

	preparedGetCurrentJobs =
		DatabaseUtility::prepareStatement(connection, "SELECT * FROM jobs.currentjobs");

	preparedDeleteCurrentJob = DatabaseUtility::prepareStatement(
		connection, "DELETE FROM jobs.currentjobs WHERE jobid = ?");

	preparedAddFailedJob =
		DatabaseUtility::prepareStatement(connection, "INSERT INTO jobs.failedjobs (jobid, time, timeout, priority, url, "
													  "retries, reasonID, reasonData) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");

	preparedUploadJob = DatabaseUtility::prepareStatement(
		connection,
		"INSERT INTO jobs.jobsqueue (constant, jobid, priority, url, retries, timeout) VALUES (1, uuid(), ?, ?, ?, ?)");

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
		CassUuid id;

		cass_value_get_uuid(cass_row_get_column_by_name(row, "jobid"), &id);
		job.url = DatabaseUtility::getString(row, "url");
		job.priority = DatabaseUtility::getInt64(row, "priority");
		job.retries = DatabaseUtility::getInt32(row, "retries");
		job.timeout = DatabaseUtility::getInt64(row, "timeout");
		cass_statement_free(query);
		cass_future_free(resultFuture);

		// Add job to list of current jobs.
		long long currTime = addCurrentJob(id, job);
		if (errno != 0)
		{
			return job;
		}

		// Delete the job that is returned.
		deleteTopJob(id, job.priority);
		if (errno != 0)
		{
			// Do not delete current job, to prevent a newer version of the job from being accidentally deleted.
			return job;
		}

		char jobid[CASS_UUID_STRING_LENGTH];
		cass_uuid_string(id, jobid);

		// Return result.
		
		job.jobid = jobid;
		job.time = currTime;
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

	numberOfJobs--;

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

long long DatabaseConnection::getCurrentJobTime(std::string jobid)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(preparedGetCurrentJob);
	CassUuid id;
	cass_uuid_from_string(jobid.c_str(), &id);

	cass_statement_bind_uuid_by_name(query, "jobid", id);

	CassFuture *resultFuture = cass_session_execute(connection, query);

	if (cass_future_error_code(resultFuture) == CASS_OK)
	{
		// Retrieve the result.
		const CassResult *result = cass_future_get_result(resultFuture);
		if (cass_result_row_count(result) >= 1)
		{
			const CassRow *row = cass_result_first_row(result);
			long long time = DatabaseUtility::getInt64(row, "time");
			cass_statement_free(query);
			cass_future_free(resultFuture);
			return time;
		}
		return -1;
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
		return -1;
	}
}

long long DatabaseConnection::addCurrentJob(Job job)
{
	CassUuid jobid;
	cass_uuid_from_string(job.jobid.c_str(), &jobid);
	return addCurrentJob(jobid, job);
}

long long DatabaseConnection::addCurrentJob(CassUuid id, Job job)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(preparedAddCurrentJob);

	cass_statement_bind_uuid_by_name(query, "jobid", id);
	long long currTime = Utility::getCurrentTimeMilliSeconds();
	cass_statement_bind_int64_by_name(query, "time", currTime);
	cass_statement_bind_int64_by_name(query, "timeout", job.timeout);
	cass_statement_bind_int64_by_name(query, "priority", job.priority);
	cass_statement_bind_string_by_name(query, "url", job.url.c_str());
	cass_statement_bind_int32_by_name(query, "retries", job.retries);	

	CassFuture *queryFuture = cass_session_execute(connection, query);

	// Statement objects can be freed immediately after being executed.
	cass_statement_free(query);

	// This will block until the query has finished.
	CassError rc = cass_future_error_code(queryFuture);

	if (rc != 0)
	{
		const char *message;
		size_t messageLength;
		cass_future_error_message(queryFuture, &message, &messageLength);
		printf("Unable to add current job: '%.*s'\n", (int)messageLength, message);
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

void DatabaseConnection::addFailedJob(FailedJob job)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(preparedAddFailedJob);

	CassUuid jobid;
	cass_uuid_from_string(job.jobid.c_str(), &jobid);

	cass_statement_bind_uuid_by_name(query, "jobid", jobid);
	cass_statement_bind_int64_by_name(query, "time", job.time);
	cass_statement_bind_int64_by_name(query, "timeout", job.timeout);
	cass_statement_bind_int64_by_name(query, "priority", job.priority);
	cass_statement_bind_string_by_name(query, "url", job.url.c_str());
	cass_statement_bind_int32_by_name(query, "retries", job.retries);
	cass_statement_bind_int32_by_name(query, "reasonID", job.reasonID);
	cass_statement_bind_string_by_name(query, "reasonData", job.reasonData.c_str());

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
	long long timeNow = Utility::getCurrentTimeSeconds();
	if (timeNow - timeLastRecount > RECOUNT_WAIT_TIME)
	{
		errno = 0;
		CassStatement *query = cass_prepared_bind(preparedAmountOfJobs);
		CassFuture *resultFuture = cass_session_execute(connection, query);
		if (cass_future_error_code(resultFuture) == CASS_OK)
		{
			// Retrieve the result.
			const CassResult *result = cass_future_get_result(resultFuture);
			const CassRow *row = cass_result_first_row(result);
			long long count = DatabaseUtility::getInt64(row, "count");
			cass_statement_free(query);
			cass_future_free(resultFuture);
			timeLastRecount = Utility::getCurrentTimeSeconds();
			numberOfJobs = count;
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
	}
	return numberOfJobs;
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

void DatabaseConnection::uploadJob(std::string url, long long priority, int retries, long long timeout, bool newJob)
{
	errno = 0;
	CassStatement *query = cass_prepared_bind(preparedUploadJob);

	long long resultPriority;
	if (newJob)
	{
		long long currentTime = Utility::getCurrentTimeMilliSeconds();
		resultPriority = currentTime - priority;
	}
	else
	{
		resultPriority = priority;
	}
	
	cass_statement_bind_int64_by_name(query, "priority", resultPriority);
	cass_statement_bind_string_by_name(query, "url", url.c_str());
	cass_statement_bind_int32_by_name(query, "retries", retries);
	cass_statement_bind_int64_by_name(query, "timeout", timeout);

	CassFuture *queryFuture = cass_session_execute(connection, query);

	numberOfJobs++;

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

void DatabaseConnection::updateCurrentJobs()
{
	while (true)
	{
		usleep(UPDATE_JOBS_TIMEOUT);
		CassStatement *query = cass_prepared_bind(preparedGetCurrentJobs);
		CassFuture *resultFuture = cass_session_execute(connection, query);

		if (cass_future_error_code(resultFuture) == CASS_OK)
		{
			const CassResult *result = cass_future_get_result(resultFuture);

			// Add the methods found as the result of the query.
			CassIterator *iterator = cass_iterator_from_result(result);
			long long currentTime = Utility::getCurrentTimeMilliSeconds();
			while (cass_iterator_next(iterator))
			{
				const CassRow *row = cass_iterator_get_row(iterator);
				CassUuid jobid;
				cass_value_get_uuid(cass_row_get_column_by_name(row, "jobid"), &jobid);
				updateCurrentJob(jobid, retrieveCurrentJob(row), currentTime);
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
			fprintf(stderr, "Unable to get current jobs: '%.*s'\n", (int)messageLength, message);
			errno = ENETUNREACH;
		}

		cass_statement_free(query);
		cass_future_free(resultFuture);
	}
}

void DatabaseConnection::updateCurrentJob(CassUuid jobid, Job job, long long currentTime)
{
	if (job.time + job.timeout < currentTime)
	{
		FailedJob failedJob(job, 2, "Timed out at: " + std::to_string(currentTime));
		addFailedJob(failedJob);
		deleteCurrentJob(jobid);
		if (job.retries < MAX_JOB_RETRIES)
		{
			uploadJob(job.url, job.priority, job.retries + 1, job.timeout, false);
		}
	}
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

/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include <string>
#include <cassandra.h>

#define IP "cassandra"
#define DBPORT 8002
#define MAX_THREADS 16

/// <summary>
/// Handles interaction with database when dealing with job requests.
/// </summary>
class DatabaseConnection
{
public:
	/// <summary>
	/// Connect to the database.
	/// </summary>
	virtual void connect(std::string ip, int port);

	/// <summary>
	/// Adds a job to the database given the url to a repository and a priority, 
	/// together with number of previous tries and a timeout.
	/// </summary>
	virtual void uploadJob(std::string url, long long priority, int retries, long long timeout);

	/// <summary>
	/// Retrieves the url of the first job in the jobs table and returns it.
	/// </summary>
	/// <returns> Tuple of URL, time job was given out and allotted timeout. </returns>
	virtual std::tuple<std::string, std::string, long long, long long> getTopJob();

	/// <summary>
	/// Returns the amount of jobs in the jobs table.
	/// </summary>
	virtual int getNumberOfJobs();

	/// <summary>
	/// Returns the current crawl ID in the database.
	/// </summary>
	virtual int getCrawlID();

	/// <summary>
	/// Sets the crawl ID in the database to the given value.
	/// </summary>
	virtual void setCrawlID(int id);

private:
	/// <summary>
	/// Deletes the first job in the jobs table given its jobid and priority.
	/// </summary>
	void deleteTopJob(CassUuid id, cass_int64_t priority);

	/// <summary>
	/// Adds a job to the currentjobs table.
	/// </summary>
	void addCurrentJob(CassUuid id, long long currTime, long long timeout, long long priority, std::string url,
					   int retries);

	/// <summary>
	/// Adds a job to the failedjobs table.
	/// </summary>
	void addFailedJob(CassUuid id, long long currTime, long long priority, std::string url, int retries, int reasonID,
					  std::string reasonData);

	/// <summary>
	/// Creates prepared queries for later use.
	/// </summary>
	void setPreparedStatements();

	/// <summary>
	/// Prepares a specified statement (query) to be executed later.
	/// </summary>
	/// <param name="query"> A string containing the CQL-query to be executed later. </param>
	/// <returns> The constant prepared statement that allows us to execute the query given as input. </returns>
	const CassPrepared *prepareStatement(std::string query);

	/// <summary>
	/// The connection with the database.
	/// <summary>
	CassSession *connection;

	const CassPrepared *preparedGetTopJob;
	const CassPrepared *preparedDeleteTopJob;
	const CassPrepared *preparedAddCurrentJob;
	const CassPrepared *preparedAmountOfJobs;
	const CassPrepared *preparedUploadJob;
	const CassPrepared *preparedCrawlID;
	const CassPrepared *preparedUpdateCrawlID;
};


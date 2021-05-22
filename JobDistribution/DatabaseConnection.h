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
#define MAX_THREADS 32

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
        /// Uploads a job to the database given the url to a repository and a priority.
        /// </summary>
	virtual void uploadJob(std::string url, int priority);

	/// <summary>
        /// Retrieves the url of the first job in the jobs table..
        /// </summary>
        std::string getTopJob();

	/// <summary>
        /// Returns the amount of jobs in the jobs table.
        /// </summary>
        int getNumberOfJobs();
private:

	/// <summary>
        /// Deletes the first job in the jobs table.
        /// </summary>
	void deleteTopJob(CassUuid id);

	/// <summary>
	/// Creates prepared queries for later use.
	/// </summary>
	void setPreparedStatements();

	/// <summary>
	/// The connection with the database.
	/// <summary>
	CassSession *connection;

	const CassPrepared *preparedGetTopJob;
	const CassPrepared *preparedDeleteTopJob;
	const CassPrepared *preparedAmountOfJobs;
	const CassPrepared *preparedUploadJob;
};


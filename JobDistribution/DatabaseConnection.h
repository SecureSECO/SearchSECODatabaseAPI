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
	/// Return the url of the first job in jobs table if the number of jobs in the table is high enough, or return a string that indicates that the job is to crawl new repositories if the number of jobs is not high enough.
	/// </summary>
	virtual std::string getJob();

	virtual void uploadJob(std::string url);
private:
	/// <summary>
        /// Retrieves the url of the first job in the jobs table..
        /// </summary>
	std::string getTopJob();

	/// <summary>
        /// Deletes the first job in the jobs table.
        /// </summary>
	void deleteTopJob(CassUuid id);

	/// <summary>
        /// Returns the amount of jobs in the jobs table.
        /// </summary>
	int getNumberOfJobs();

	/// <summary>
	/// The connection with the database.
	/// <summary>
	CassSession *connection;

};


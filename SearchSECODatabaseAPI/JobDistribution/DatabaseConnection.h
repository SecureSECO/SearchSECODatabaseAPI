/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include "JobTypes.h"

#include <string>
#include <cassandra.h>

#define IP "cassandra"
#define DBPORT 8002
#define UPDATE_JOBS_TIMEOUT 300000000 // 5 minutes.
#define MAX_JOB_RETRIES 3
#define RECOUNT_WAIT_TIME 600

using namespace jobTypes;

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
	virtual void uploadJob(std::string url, long long priority, int retries, long long timeout, bool newJob);

	/// <summary>
	/// Retrieves the url of the first job in the jobs table and returns it.
	/// </summary>
	/// <returns> Job object containing data on the top job. </returns>
	virtual Job getTopJob();

	/// <summary>
	/// Retrieves a job with matching jobid in the currentjobs table.
	/// </summary>
	/// <returns>
	/// Job object containing information on job, or nullptr if
	/// no match was present.
	/// </returns>
	virtual Job getCurrentJob(std::string jobid);

	/// <summary>
	/// Adds a job to the currentjobs table.
	/// </summary>
	virtual long long addCurrentJob(Job job);

	/// <summary>
	/// Adds a job to the failedjobs table.
	/// </summary>
	virtual void addFailedJob(FailedJob job);

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

	/// <summary>
	/// Regularly updates the current jobs to check for a timeout.
	/// </summary>
	virtual void updateCurrentJobs();

private:
	/// <summary>
	/// Deletes the first job in the jobs table given its jobid and priority.
	/// </summary>
	void deleteTopJob(CassUuid id, cass_int64_t priority);

	/// <summary>
	/// Deletes the job in the currentjobs table given its jobid.
	/// </summary>
	void deleteCurrentJob(CassUuid id);

	/// <summary>
	/// Adds a job to the currentjobs table.
	/// </summary>
	long long addCurrentJob(CassUuid id, Job job);

	/// <summary>
	/// Retrieves the job from the given row.
	/// </summary>
	Job retrieveCurrentJob(const CassRow *row);

	/// <summary>
	/// Creates prepared queries for later use.
	/// </summary>
	void setPreparedStatements();

	/// <summary>
	/// Moves a job with passed timeout to the failed jobs.
	/// </summary>
	virtual void updateCurrentJob(CassUuid jobid, Job job, long long currentTime);

	/// <summary>
	/// The connection with the database.
	/// <summary>
	CassSession *connection;

	int numberOfJobs;
	long long timeLastRecount = -1;

	const CassPrepared *preparedGetTopJob;
	const CassPrepared *preparedDeleteTopJob;
	const CassPrepared *preparedAddCurrentJob;
	const CassPrepared *preparedGetCurrentJob;
	const CassPrepared *preparedGetCurrentJobs;
	const CassPrepared *preparedDeleteCurrentJob;
	const CassPrepared *preparedAddFailedJob;
	const CassPrepared *preparedAmountOfJobs;
	const CassPrepared *preparedUploadJob;
	const CassPrepared *preparedCrawlID;
	const CassPrepared *preparedUpdateCrawlID;
};


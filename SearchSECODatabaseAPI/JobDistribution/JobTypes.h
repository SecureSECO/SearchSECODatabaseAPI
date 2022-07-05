/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include <string>

namespace jobTypes
{
	/// <summary>
	/// Mirrors an entry of the currentjobs table in the database.
	/// </summary>
	struct Job
	{
	  public:
		std::string jobid;
		long long time;
		long long timeout;
		long long priority;
		std::string url;
		int retries;

		Job(std::string jobid, long long timeout, long long priority, std::string url, int retries)
			: jobid(jobid), timeout(timeout), priority(priority), url(url), retries(retries)
		{
		}

		Job()
		{
		}
	};

	/// <summary>
	/// Mirrors an entry of the failedjobs table in the database. Difference with
	/// Job is the addition of a reasonID and reasonData, to indicate
	/// a reason for failure.
	/// </summary>
	struct FailedJob
	{
	  public:
		std::string jobid;
		long long time;
		long long timeout;
		long long priority;
		std::string url;
		int retries;
		int reasonID;
		std::string reasonData;

		/// <summary>
		/// Constructs a failed job given a normal job and the reason.
		/// </summary>
		/// <param name="job"> The job this failed job is based on. </param>
		/// <param name="reasonID"> The id of the failure reason. </param>
		/// <param name="reasonData"> The data of the reason. </param>
		FailedJob(Job job, int reasonID, std::string reasonData)
			: jobid(job.jobid), time(job.time), timeout(job.timeout), priority(job.priority), url(job.url),
			  retries(job.retries), reasonID(reasonID), reasonData(reasonData)
		{
		}
	};
} // namespace jobTypes
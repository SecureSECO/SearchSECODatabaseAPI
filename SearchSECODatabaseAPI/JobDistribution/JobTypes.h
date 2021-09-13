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
	};
}
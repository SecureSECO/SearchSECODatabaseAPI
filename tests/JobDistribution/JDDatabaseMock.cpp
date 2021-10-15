/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "JobTypes.h"
#include <string>
#include <gmock/gmock.h>

using namespace jobTypes;

/// <summary>
/// Handles interaction with database.
/// </summary>
class MockJDDatabase : public DatabaseConnection
{
public:
	MOCK_METHOD(void, connect, (std::string ip, int port), ());
	MOCK_METHOD(void, uploadJob, (std::string url, long long priority, int retries, long long timeout, bool newJob), ());
	MOCK_METHOD(Job, getTopJob, (), ());
	MOCK_METHOD(Job, getCurrentJob, (std::string jobid), ());
	MOCK_METHOD(long long, addCurrentJob, (Job job), ());
	MOCK_METHOD(int, getNumberOfJobs, (), ());
	MOCK_METHOD(int, getCrawlID, (), ());
	MOCK_METHOD(void, setCrawlID, (int id), ());
};


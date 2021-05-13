/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "gmock/gmock.h"
#include <string>

using namespace types;

/// <summary>
/// Handles interaction with database.
/// </summary>
class MockJDDatabase : public DatabaseConnection
{
public:
	MOCK_METHOD(void, connect, (std::string ip, int port), ());
	MOCK_METHOD(std::string, getJob, (), ());
	MOCK_METHOD(void, uploadJob, (std::string url), ());
};


/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "gmock/gmock.h"
#include "Types.h"
#include <string>

using namespace types;

/// <summary>
/// Handles interaction with database.
/// </summary>
class MockDatabase :: public DatabaseHandler
{
public:
    MOCK_METHOD(void, Connect, (), (override));
    MOCK_METHOD(void, AddProject, (Project project), (override));
    MOCK_METHOD(void, AddMethod, (MethodIn method, Project project), (override));
    MOCK_METHOD(std::vector<MethodOut>, HashToMethods, (std::string hash), (override));
    MOCK_METHOD(bool, AddMethodByAuthor, (CassUuid authorID, MethodIn method, Project project), (override));
    MOCK_METHOD(MethodOut, GetMethod, (const CassRow* row), (override));
    MOCK_METHOD(CassUuid, GetAuthorID, (Author author), (override));
};

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
class MockDatabase : public DatabaseHandler
{
public:
    MOCK_METHOD(void, connect, (), ());
    MOCK_METHOD(void, addProject, (Project project), ());
    MOCK_METHOD(void, addMethod, (MethodIn method, Project project), ());
    MOCK_METHOD(std::vector<MethodOut>, hashToMethods, (std::string hash), ());
};

/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "gmock/gmock.h"
#include "Types.h"
#include "DatabaseHandler.h"
#include <string>

using namespace types;

/// <summary>
/// Handles interaction with database.
/// </summary>
class MockDatabase : public DatabaseHandler
{
public:
	MOCK_METHOD(void, connect, (std::string ip, int port), ());
	MOCK_METHOD(void, addProject, (Project project), ());
	MOCK_METHOD(void, addMethod, (MethodIn method, Project project), ());
	MOCK_METHOD(std::vector<MethodOut>, hashToMethods, (std::string hash), ());
	MOCK_METHOD(std::string, authorToId, (Author author), ());
	MOCK_METHOD(Author, idToAuthor, (std::string id), ());
	MOCK_METHOD(std::vector<MethodId>, authorToMethods, (std::string authorId));
};

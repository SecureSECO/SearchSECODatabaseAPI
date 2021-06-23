/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "Types.h"
#include "DatabaseHandler.h"

#include <string>
#include <gmock/gmock.h>

using namespace types;

/// <summary>
/// Handles interaction with database.
/// </summary>
class MockDatabase : public DatabaseHandler
{
public:
	MOCK_METHOD(void, connect, (std::string ip, int port), ());
	MOCK_METHOD(bool, addProject, (ProjectIn project), ());
	MOCK_METHOD(void, addHashToProject, (ProjectIn project, int index), ());
	MOCK_METHOD(void, addMethod, (MethodIn method, ProjectIn project, long long prevVersion, long long parserVersion, bool newProject), ());
	MOCK_METHOD(ProjectOut, searchForProject, (ProjectID projectID, Version version), ());
	MOCK_METHOD(ProjectOut, prevProject, (ProjectID projectID), ());
	MOCK_METHOD(std::vector<Hash>, updateUnchangedFiles,
				(std::vector<Hash> hashes, std::vector<std::string> files, ProjectIn project, long long prevVersion),
				());
	MOCK_METHOD(std::vector<MethodOut>, hashToMethods, (std::string hash), ());
	MOCK_METHOD(std::string, authorToID, (Author author), ());
	MOCK_METHOD(Author, idToAuthor, (std::string id), ());
	MOCK_METHOD(std::vector<MethodID>, authorToMethods, (std::string authorID));
};

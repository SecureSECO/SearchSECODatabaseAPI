/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include "DatabaseHandler.h"
#include "Definitions.h"
#include <mutex>
#include <queue>

#define PROJECT_DATA_SIZE 8
#define METHOD_DATA_MIN_SIZE 5
#define HEX_CHARS "0123456789abcdef"
#define UUID_REGEX "[0-9a-fA-F]{8}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{12}"
#define MAX_RETRIES 3
#define HASHES_MAX_SIZE 1000
#define FILES_MAX_SIZE 500

/// <summary>
/// Handles requests towards database.
/// </summary>
class DatabaseRequestHandler
{
public:
	DatabaseRequestHandler(DatabaseHandler *database, std::string ip = IP, int port = DBPORT);

	/// <summary>
	/// Handles requests which want to add one project with their corresponding methods to the database.
	/// </summary>
	/// <param name="request">
	/// The request made by the user. It has the following format (where | is defined as FIELD_DELIMITER_CHAR):
	/// "projectID|version|license|project_name|url|author_name|author_mail'\n'
	///  method1_hash|method1_name|method1_fileLocation|method1_lineNumber|method1_numberOfAuthors|
	///  method1_author1_name|method1_author1_mail|<other authors>'\n'<method2_data>'\n'...'\n'<methodN_data>".
	/// </param>
	/// <returns>
	/// Response towards user after processing the request successfully.
	/// </returns>
	std::string handleUploadRequest(std::string request);

	/// <summary>
	/// Handles requests wanting to obtain methods with certain hashes.
	/// </summary>
	/// <param name="request">
	/// The request made by the user, having the following format
	/// (where '\n' is defined as ENTRY_DELIMITER_CHAR):
	/// "hash_1'\n'hash_2'\n'...'\n'hash_N".
	/// </param>
	/// <returns>
	/// The methods which contain hashes equal to one within the request, in string format.
	/// </returns>
	std::string handleCheckRequest(std::string request);

	/// <summary>
	/// Handles requests wanting to obtain methods with certain hashes.
	/// </summary>
	/// <param name="hashes">
	/// The list of hashes we want to check for.
	/// </param>
	/// <returns>
	/// The methods which contain hashes equal to one within the request, in string format.
	/// </returns>
	std::string handleCheckRequest(std::vector<Hash> hashes);

	/// <summary>
	/// Handles requests wanting to first check for matches with existing methods in other projects,
	/// after which it adds the project itself to the database.
	/// </summary>
	/// <param name="request">
	/// The request made by the user, having the following format
	/// (where | and '\n' are defined as FIELD_DELIMITER_CHAR and ENTRY_DELIMITER_CHAR respectively):
	/// "projectID|version|license|project_name|url|author_name|author_mail'\n'
	///  method1_hash|method1_name|method1_fileLocation|method1_lineNumber|method1_numberOfAuthors|
	///  method1_author1_name|method1_author1_mail|...|method1_authorM_name|method1_authorM_mail'\n'
	///  <method2_data>'\n'...'\n'<methodN_data>".
	/// </param>
	/// <returns>
	/// The methods which contain hashes equal to the one of the hashes
	/// of the methods within the request, in string format.
	/// </returns>
	std::string handleCheckUploadRequest(std::string request);

	/// <summary>
	/// Handles requests wanting to obtain project data from the database given their projectID and version.
	/// </summary>
	/// <param name="request">
	/// The request made by the user which has the following format
	/// (where | and '\n' are defined as FIELD_DELIMITER_CHAR and ENTRY_DELIMITER_CHAR respectively):
	/// "projectID_1|version_1'\n'...'\n'projectID_M|version_M".
	/// </param>
	/// <returns>
	/// The relevant projects found in the database in string format as follows:
	/// "projectID_1|version_1|license_1|project_name_1|url_1|owner_id1'\n'"
	/// "<project2_data>'\n'...'\n'<projectN_data>".
	/// </returns>
	std::string handleExtractProjectsRequest(std::string request);

	/// Handles a requests for retrieving the ids by the give authors.
	/// </summary>
	/// <param nam'="request">
	/// The request that contains the authors with the following format
	/// (where | and '\n' are defined as FIELD_DELIMITER_CHAR and ENTRY_DELIMITER_CHAR respectively):
	/// name_1|mail_1'\n'name_2|mail_2'\n'...
	/// </param>
	/// <returns>
	/// A string with the author ids retrieved from the database with the following format:
	/// name_1|mail_1|id_1'\n'name_2|mail_2|id_2'\n'...
	/// </returns>
	std::string handleGetAuthorIDRequest(std::string request);

	/// <summary>
	/// Handles a requests for retrieving the authors by the given ids.
	/// </summary>
	/// <param nam'="request">
	/// The request that contains the author ids with the following format
	/// (where '\n' is defined as ENTRY_DELIMITER_CHAR):
	/// id_1'\n'id_2'\n'...
	/// </param>
	/// <returns>
	/// A string with the authors retrieved from the database with the following format
	/// (where | is defined as FIELD_DELIMITER_CHAR):
	/// name_1|mail_1|id_1'\n'name_2|mail_2|id_2'\n'...
	/// </returns>
	std::string handleGetAuthorRequest(std::string request);

	/// <summary>
	/// Handles requests wanting to obtain methods with certain authors.
	/// </summary>
	/// <param name="request">
	/// The request made by the user, having the following format
	/// (where '\n' is defined as ENTRY_DELIMITER_CHAR):
	/// "authorId_1'\n'authorId_2'\n'...".
	/// </param>
	/// <returns>
	/// The methods that the given author has worked on with the following format:
	/// (where | is defined as FIELD_DELIMITER_CHAR):
	/// authorId_1|hash_1|projectId_1|version_1'\n'authorId_2|hash_2|projectId_2|version_2'\n'...
	/// </returns>
	std::string handleGetMethodsByAuthorRequest(std::string request);

private:
	/// <summary>
	/// Converts a request to a Project (defined in Types.h).
	/// </summary>
	/// <param name="request">
	/// The relevant data to create the Project. It has the following format:
	/// "projectID|version|license|project_name|url|author_name|author_mail".
	/// </param>
	/// <returns>
	/// A Project containing all data as provided within request.
	/// </returns>
	ProjectIn requestToProject(std::string request);

	/// <summary>
	/// Converts a data entry to a Method (defined in Types.h).
	/// </summary>
	/// <param name="dataEntry">
	/// The relevant data to create the Method. It has the following format
	/// (where | is defined as FIELD_DELIMITER_CHAR):
	/// "method_hash|method_name|method_fileLocation|number_of_authors|
	///  method_author1_name|method_author1_mail|...|method_authorN_name|method_authorN_mail".
	/// </param>
	/// <returns>
	/// A method containing all data as provided in input.
	/// </returns>
	MethodIn dataEntryToMethod(std::string dataEntry);

	/// <summary>
	/// Retrieves the hashes within a request.
	/// </summary>
	/// <param name="request">
	/// Represents the request made by the user. It has the following format
	/// (where | and '\n' are defined as FIELD_DELIMITER_CHAR and ENTRY_DELIMITER_CHAR respectively):
	/// "projectID|version|license|project_name|url|author_name|author_mail'\n'
	///  method1_hash|method1_name|method1_fileLocation|method1_lineNumber|method1_numberOfAuthors|
	///  method1_author1_name|method1_author1_mail|<other authors>'\n'<method2_data>'\n'...'\n'<methodN_data>".
	/// </param>
	/// <returns>
	/// The hashes of the methods given in the requests.
	/// </returns>
	std::vector<Hash> requestToHashes(std::string request);

	/// <summary>
	/// Checks if a hash is valid.
	/// </summary>
	/// <param name="hash">
	/// The hash to be checked.
	/// </param>
	/// <returns>
	/// Boolean indicating the validity of the given hash.
	/// </returns>
	bool isValidHash(Hash hash);

	/// <summary>
	/// Converts methods to a string by placing special delimiters between fields and between entries.
	/// </summary>
	/// <returns>
	/// A string consisting of all provided methods, separated by 'dataDelimiter' to recognise different fields and
	/// separated by 'methodDelimiter' for separate methods.
	/// </returns>
	std::string methodsToString(std::vector<MethodOut> methods, char dataDelimiter, char methodDelimiter);

	/// <summary>
	/// Converts projects to a string by placing special delimiters between fields and between entries.
	/// </summary>
	/// <returns>
	/// A string consisting of all provided projects, separated by 'dataDelimiter' to recognise different fields and
	/// separated by 'projectDelimiter' for separate projects..
	/// </returns>
	std::string projectsToString(std::vector<ProjectOut> projects, char dataDelimiter, char projectDelimiter);

	/// <summary>
	/// Retrieves the methods corresponding to the hashes given as input using the database.
	/// </summary>
	/// <param name="hashes">
	/// A vector of hashes.
	/// </param>
	/// <returns>
	/// All methods in the database with a hash equal to one in 'hashes'.
	/// </returns>
	std::vector<MethodOut> getMethods(std::vector<Hash> hashes);

	/// <summary>
	/// Retrieves the projects corresponding to the projectKeys given as input (in a queue) using the database.
	/// </summary>
	/// <param name="keys">
	/// A queue of projectKeys, which are pairs of projectIDs and versions.
	/// </param>
	/// <returns>
	/// All projects in the database corresponding to one of the keys in the queue 'keys'.
	/// </returns>
	std::vector<ProjectOut> getProjects(std::queue<std::pair<ProjectID, Version>> keys);

	/// <summary>
	/// Handles a single thread of checking hashes with the database.
	/// </summary>
	/// <param name="hashes">
	/// The queue with hashes that have to be checked.
	/// </param>
	/// <param name="queueLock">
	/// The lock for the queue with hashes.
	/// </param>
	/// <returns>
	/// The methods found by a single thread inside a vector.
	/// </returns>
	std::vector<MethodOut> singleHashToMethodsThread(std::queue<Hash> &hashes, std::mutex &queueLock);

	/// <summary>
	/// Handles a single thread of checking hashes with the database.
	/// </summary>
	/// <param name="hashes">
	/// The queue with hashes that have to be checked.
	/// </param>
	/// <param name="queueLock">
	/// The lock for the queue with hashes.
	/// </param>
	/// <returns>
	/// The projects found by a single thread inside a vector.
	/// </returns>
	std::vector<ProjectOut> singleSearchProjectThread(std::queue<std::pair<ProjectID, Version>> &projectKeyQueue, std::mutex &queueLock);

	/// <summary>
	/// Handles a single thread of uploading methods to the database.
	/// </summary>
	/// <param name="hashes">
	/// The queue with methods that have to be added to the databse.
	/// </param>
	/// <param name="queueLock">
	/// The lock for the queue with methods.
	/// </param>
	/// <param name="project">
	/// The project the methods are part of.
	/// </param>
	/// <returns></returns>
	void singleUploadThread(std::queue<MethodIn> &methods, std::mutex &queueLock, ProjectIn project, long long prevVersion);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="hashFiles"></param>
	/// <param name="queueLock"></param>
	/// <param name="project"></param>
	/// <param name="prevVersion"></param>
	/// <returns></returns>
	std::vector<Hash>
	singleUpdateUnchangedFilesThread(std::queue<std::pair<std::vector<Hash>, std::vector<std::string>>> &hashFiles,
									 std::mutex &queueLock, ProjectIn project, long long prevVersion);

	/// <summary>
	/// Parses a list of authors with ids to a string to be returned.
	/// </summary>
	/// <param name="authors">
	/// A vector of tuples containing an author and the corresponding id.
	/// </param>
	/// <returns>
	/// A string of the format:
	/// (where | and '\n' are defined as FIELD_DELIMITER_CHAR and ENTRY_DELIMITER_CHAR respectively):
	/// name_1|mail_1|id_1'\n'name_2|mail_2|id_2'\n'...
	/// <returns>
	std::string authorsToString(std::vector<std::tuple<Author, std::string>> authors);

	/// <summary>
	/// Parses a dataentry to an author.
	/// </summary>
	/// <param name="dataEntry">
	/// A string with the name and mail of an author seperated by the FIELD_DELIMITER_CHAR.
	/// </param>
	/// <returns>
	/// An author with the name and mail passed to the method.
	/// <returns>
	Author datanEntryToAuthor(std::string dataEntry);

	/// <summary>
	/// Retrieves the id corresponding to the authors given as input using the database.
	/// </summary>
	/// <param name="authors">
	/// A vector of strings representing authors.
	/// </param>
	/// <returns>
	/// A vector consisting of tuples with an author and the corresponding id.
	/// </returns>
	std::vector<std::tuple<Author, std::string>> getAuthorIDs(std::vector<Author> authors);

	/// <summary>
	/// Handles a single thread of retrieving author ids from the database.
	/// </summary>
	/// <param name="authors">
	/// The queue with authors that have to be checked.
	/// </param>
	/// <param name="queueLock">
	/// The lock for the queue with authors.
	/// </param>
	/// <returns>
	/// A vector consisting of tuples with an author and the corresponding id.
	/// </returns>
	std::vector<std::tuple<Author, std::string>> singleAuthorToIDThread(std::queue<Author> &authors,
																		std::mutex &queueLock);	

	/// <summary>
	/// Retrieves the authors corresponding to the ids given as input using the database.
	/// </summary>
	/// <param name="authorIds">
	/// A vector of author ids.
	/// </param>
	/// <returns>
	/// A vector consisting of tuples with an author and the corresponding id.
	/// </returns>
	std::vector<std::tuple<Author, std::string>> getAuthors(std::vector<std::string> authorIds);

	/// <summary>
	/// Handles a single thread of retrieving authors from the database.
	/// </summary>
	/// <param name="authorIds">
	/// The queue with author ids that have to be checked.
	/// </param>
	/// <param name="queueLock">
	/// The lock for the queue with author ids.
	/// </param>
	/// <returns>
	/// A vector consisting of tuples with an author and the corresponding id.
	/// </returns>
	std::vector<std::tuple<Author, std::string>> singleIdToAuthorThread(std::queue<std::string> &authorIds,
																		std::mutex &queueLock);

	/// <summary>
	/// Retrieves the methods worked on by one of the authors for which the id is given.
	/// </summary>
	/// <param name="authorIds">
	/// A vector of author ids.
	/// </param>
	/// <returns>
	/// All methods in the database that one of the give authors has worked on.
	/// </returns>
	std::vector<std::tuple<MethodId, std::string>> getMethodsByAuthor(std::vector<std::string> authorIds);

	/// <summary>
	/// Handles a single thread of retrieving methods by given authors.
	/// </summary>
	/// <param name="authorIds">
	/// The queue with ids of authors that have to be checked.
	/// </param>
	/// <param name="queueLock">
	/// The lock for the queue with author ids.
	/// </param>
	/// <returns>
	/// A vector consisting of tuples with a method and the corresponding id.
	/// </returns>
	std::vector<std::tuple<MethodId, std::string>> singleAuthorToMethodsThread(std::queue<std::string> &authorIds,
																			 std::mutex &queueLock);

	/// <summary>
	/// Parses a list of methods with author ids to a string to be returned.
	/// </summary>
	/// <param name="methods">
	/// A vector of tuples containing a method and the corresponding author id.
	/// </param>
	/// <returns>
	/// A string of the format
	/// (where | and '\n' are defined as FIELD_DELIMITER_CHAR and ENTRY_DELIMITER_CHAR respectively):
	/// authorId_1|hash_1|projectId_1|version_1'\n'authorId_2|hash_2|projectId_2|version_2'\n'...
	/// <returns>
	std::string methodIdsToString(std::vector<std::tuple<MethodId, std::string>> methods);

	/// <summary>
	/// Calls connect in the DatabaseHandler, if connect fails, it retries as many times as the MAX_RETRIES.
	/// If it still fails on the last retry, the function throws an exception.
	/// </summary>
	void connectWithRetry(std::string ip, int port);

	/// <summary>
	/// Tries to upload project to the database, if it fails it retries as many times as the MAX_RETRIES.
	/// If it succeeds, it returns true. If it still fails on the last retry, it returns false.
	/// </summary>
	bool tryUploadProjectWithRetry(ProjectIn project);

	/// <summary>
	/// Tries to add method to the database, if it fails it retries as many times as MAX_RETRIES.
	/// If it succeeds, it adds the method to the database and puts errno on 0.
	/// If it still fails on the last retry, it puts the errno on ENETUNREACH and returns.
	/// </summary>
	/// <param name="present">
	/// A boolean that is true if and only if the method is present in the previous version of the project.
	/// </param>
	void addMethodWithRetry(MethodIn method, ProjectIn project, long long prevVersion);

	/// <summary>
	/// 
	/// </summary>
	std::vector<Hash> updateUnchangedFilesWithRetry(std::pair<std::vector<Hash>, std::vector<std::string>> hashFile,
													ProjectIn project, long long prevVersion);

	/// <summary>
	/// Tries to get all methods with a given hash from the database, if it fails it retries as many times as MAX_RETRIES.
	/// If it succeeds, it returns the methods found in the database and puts the errno on 0.
	/// If it fails, it puts the errno on ENETUNREACH and returns an empty vector.
	/// </summary>
	std::vector<MethodOut> hashToMethodsWithRetry(Hash hash);

	/// <summary>
	/// Tries to get projects with a given version and projectID from the database, if it fails it retries like above.
	/// If it succeeds, it returns the projects and puts the errno on 0.
	/// If it fails, it returns an empty vector and puts the errno on ENETUNREACH.
	/// </summary>
	std::vector<ProjectOut> searchForProjectWithRetry(ProjectID projectID, Version version);

	/// <summary>
	/// Tries to get authorID from the database given an author, if it fails it retries like above.
	/// If it succeeds, it returns the id and puts errno on 0.
	/// If it fails, it returns an empty string and puts errno on ENETUNREACH.
	/// </summary>
	std::string authorToIdWithRetry(Author author);

	/// <summary>
	/// Tries to get author from the database given an authorID, if it fails it retries like above.
	/// If it succeeds, it returns the author and puts errno on 0.
	/// If it fails, it returns an emprt author and puts errno on ENETUNREACH.
	/// </summary>
	Author idToAuthorWithRetry(std::string id);

	/// <summary>
	/// Tries to get methods from the database given an authorID, if it fails it retries like above.
	/// If it succeeds, it returns the methods and puts errno on 0.
	/// If it fails, it returns an empty vector and puts errno on ENETUNREACH.
	/// </summary>
	std::vector<MethodId> authorToMethodsWithRetry(std::string authorId);

	DatabaseHandler *database;
};

/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include "DatabaseHandler.h"
#include "Definitions.h"
#include <mutex>
#include <tuple>
#include <queue>

#define PROJECT_DATA_SIZE 9
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
		/// "projectID|version|license|project_name|url|author_name|author_mail|parserVersion'\n'
		///  prevVersion'\n'
		///  unchangedfile1|unchangedfile2|...|unchangedfileM`\n`
		///  method1_hash|method1_name|method1_fileLocation|method1_lineNumber|method1_numberOfAuthors|
		///  method1_author1_name|method1_author1_mail|<other authors>'\n'<method2_data>'\n'...'\n'<methodN_data>".
		/// </param>
		/// <returns> Response towards user after processing the request successfully. </returns>
		std::string handleUploadRequest(std::string request);

		/// <summary>
		/// Handles requests wanting to obtain methods with certain hashes.
		/// </summary>
		/// <param name="request">
		/// The request made by the user, having the following format
		/// (where '\n' is defined as ENTRY_DELIMITER_CHAR):
		/// "hash_1'\n'hash_2'\n'...'\n'hash_N".
		/// </param>
		/// <returns> The methods which contain hashes equal to one within the request, in string format. </returns>
		std::string handleCheckRequest(std::string request);

		/// <summary>
		/// Handles requests wanting to obtain methods with certain hashes.
		/// </summary>
		/// <param name="hashes"> The list of hashes we want to check for. </param>
		/// <returns> The methods which contain hashes equal to one within the request, in string format. </returns>
		std::string handleCheckRequest(std::vector<Hash> hashes);

		/// <summary>
		/// Handles requests wanting to first check for matches with existing methods in other projects,
		/// after which it adds the project itself to the database.
		/// </summary>
		/// <param name="request">
		/// The request made by the user, having the following format
		/// (where | and '\n' are defined as FIELD_DELIMITER_CHAR and ENTRY_DELIMITER_CHAR respectively):
		/// "projectID|version|license|project_name|url|author_name|author_mail|parserVersion'\n'
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

		/// <summary>
		/// Handles requests wanting to obtain project data from a the database from a previous version given their 
		/// projectID.
		/// </summary>
		/// <param name="request">
		/// The request made by the user which has the following format
		/// (where | and '\n' are defined as FIELD_DELIMITER_CHAR and ENTRY_DELIMITER_CHAR respectively):
		/// "projectID_1'\n'...'\n'projectID_M".
		/// </param>
		/// <returns>
		/// The relevant projects found in the database in string format as follows:
		/// "projectID_1|version_1|license_1|project_name_1|url_1|owner_id1'\n'"
		/// "<project2_data>'\n'...'\n'<projectN_data>".
		/// </returns>
		std::string handlePrevProjectsRequest(std::string request);

		/// <summary>
		/// Handles a requests for retrieving the authors by the given IDs.
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
		/// "authorID_1'\n'authorID_2'\n'...".
		/// </param>
		/// <returns>
		/// The methods that the given author has worked on with the following format:
		/// (where | is defined as FIELD_DELIMITER_CHAR):
		/// authorID_1|hash_1|projectID_1|version_1'\n'authorID_2|hash_2|projectID_2|version_2'\n'...
		/// </returns>
		std::string handleGetMethodsByAuthorRequest(std::string request);

	private:
		/// <summary>
		/// Converts a request to a ProjectIn (defined in Types.h).
		/// </summary>
		/// <param name="request">
		/// The relevant data to create the Project. It has the following format:
		/// "projectID|version|license|project_name|url|author_name|author_mail|parserVersion".
		/// </param>
		/// <returns> The project containing all data as provided within request. </returns>
		ProjectIn requestToProject(std::string request);

		/// <summary>
		/// Converts a data entry to a MethodIn (defined in Types.h).
		/// </summary>
		/// <param name="dataEntry">
		/// The relevant data to create the Method. It has the following format:
		/// "method_hash|method_name|method_fileLocation|number_of_authors|
		///  method_author1_name|method_author1_mail|...|method_authorN_name|method_authorN_mail".
		/// </param>
		/// <returns> A method containing all data as provided in input. </returns>
		MethodIn dataEntryToMethod(std::string dataEntry);

		/// <summary>
		/// Retrieves the hashes within a request.
		/// </summary>
		/// <param name="request">
		/// Represents the request made by the user. It has the following format:
		/// "projectID|version|license|project_name|url|author_name|author_mail|parserVersion'\n'
		///  method1_hash|method1_name|method1_fileLocation|method1_lineNumber|method1_numberOfAuthors|
		///  method1_author1_name|method1_author1_mail|<other authors>'\n'<method2_data>'\n'...'\n'<methodN_data>".
		/// </param>
		/// <returns> The hashes of the methods given in the requests. </returns>
		std::vector<Hash> requestToHashes(std::string request);

		/// <summary>
		/// Checks if a hash is valid.
		/// </summary>
		/// <param name="hash"> The hash to be checked. </param>
		/// <returns> Boolean indicating the validity of the given hash. </returns>
		bool isValidHash(Hash hash);

		/// <summary>
		/// Converts methods to a string by placing special delimiters between fields and between entries.
		/// </summary>
		/// <param name="methods"> The methods to be converted to a string. </param>
		/// <param name="dataDelimiter"> Delimiter to separate different fields in a method. </param>
		/// <param name="methodDelimiter"> Delimiter to separate different methods. </param>
		/// <returns> 
		/// A string consisting of all provided methods by means of separation of data elements and methods. 
		/// </returns>
		std::string methodsToString(std::vector<MethodOut> methods, char dataDelimiter, char methodDelimiter);
	
		/// <summary>
		/// Converts projects to a string by placing special delimiters between fields and between entries.
		/// </summary>
		/// <param name="projects"> The projects to be converted to a string. </param>
		/// <param name="dataDelimiter"> Delimiter to separate different fields in a method. </param>
		/// <param name="projectDelimiter"> Delimiter to separate different projects. </param>
		/// <returns>
		/// A string consisting of all provided projects by means of separation of data elements and projects.
		/// </returns>
		std::string projectsToString(std::vector<ProjectOut> projects, char dataDelimiter, char projectDelimiter);

		/// <summary>
		/// Retrieves the methods corresponding to the hashes given as input using the database.
		/// </summary>
		/// <param name="hashes"> A vector of hashes. </param>
		/// <returns> All methods in the database with a hash equal to one in 'hashes'. </returns>
		std::vector<MethodOut> getMethods(std::vector<Hash> hashes);

		/// <summary>
		/// Retrieves the projects corresponding to the projectKeys given as input (in a queue) using the database.
		/// </summary>
		/// <param name="keys"> A queue of pairs of projectIDs and versions. </param>
		/// <returns> All projects in the database corresponding to one of the keys in the queue 'keys'. </returns>
		std::vector<ProjectOut> getProjects(std::queue<std::pair<ProjectID, Version>> keys);

	
		/// <summary>
		/// Retrieves the previous projects from the database corresponding to some queue.
		/// </summary>
		/// <param name="projectQueue"> A queue of projectIDs. </param>
		/// <returns> The latest version of given projects. </returns>
		std::vector<ProjectOut> getPrevProjects(std::queue<ProjectID> projectQueue);

		/// <summary>
		/// Handles a single thread of checking hashes with the database.
		/// </summary>
		/// <param name="hashes"> The queue with hashes that have to be checked. </param>
		/// <param name="queueLock"> The lock for the queue. </param>
		/// <returns> The methods found by a single thread inside a vector. </returns>
		std::vector<MethodOut> singleHashToMethodsThread(std::queue<Hash> &hashes, std::mutex &queueLock);

		/// <summary> Handles a single thread of checking hashes with the database. </summary>
		/// <param name="projectKeyQueue"> The queue with pairs of projectIDs and versions that have to be checked. </param>
		/// <param name="queueLock"> The lock for the queue. </param>
		/// <returns> The projects found by a single thread inside a vector. </returns>
		std::vector<ProjectOut> singleSearchProjectThread(std::queue<std::pair<ProjectID, Version>> &projectKeyQueue,
														  std::mutex &queueLock);

		/// <summary>
		/// Handles a single thread of checking hashes (of the previous projects for the given versions) with the database.
		/// </summary>
		/// <param name="hashes"> The queue with hashes that have to be checked. </param>
		/// <param name="queueLock"> The lock for the queue. </param>
		/// <returns> The latest version of projects found by a single thread inside a vector. </returns>
		std::vector<ProjectOut> singlePrevProjectThread(std::queue<ProjectID> &projectIDs, std::mutex &queueLock);

		/// <summary>
		/// Handles the threads used to upload methods to the database.
		/// </summary>
		/// <param name="project"> The project the methods are part of. </param>
		/// <param name="methodQueue"> The queue of methods that have to be uploaded. </param>
		/// <param name="newProject"> Indication of the project being new or not. </param>
		/// <param name="prevProject"> The previous version of the project. </param>
		/// <param name="unchangedFiles"> 
		/// The files that did not change compared to the previous version of the project.
		/// </param>
		void handleUploadThreads(ProjectIn project, std::queue<MethodIn> methodQueue, bool newProject,
								 ProjectOut prevProject, std::vector<File> unchangedFiles);

		/// <summary>
		/// Handles a single thread of uploading methods to the database.
		/// </summary>
		/// <param name="methods"> The queue with methods that have to be added to the database. </param>
		/// <param name="queueLock"> The lock for the queue with methods. </param>
		/// <param name="project"> The project the methods are part of. </param>
		/// <param name="prevVersion"> The previous version of the project. </param>
		/// <param name="parserVersion"> The version of the parser. </param>
		/// <param name="newProject"> Indication of the project being new or not. </param>
		void singleUploadThread(std::queue<MethodIn> &methods, std::mutex &queueLock, ProjectIn project,
								long long prevVersion, long long parserVersion, bool newProject);

		/// <summary>
		/// A general template for a query to be performed with retries.
		/// </summary>
		/// <typeparam name="T"> 
		/// The output of the query. If no output is given, an empty tuple (std::tuple<>) is used.
		/// </typeparam>
		/// <param name="function"> The corresponding query to be performed a single time. </param>
		/// <returns> Output of the query, which may be an empty tuple. </returns>
		template <class T> T queryWithRetry(std::function<T()> function);

		/// <summary>
		/// Handles the threads used to update methods in unchanged files.
		/// </summary>
		/// <param name="project"> The project to be added/updated to the database. </param>
		/// <param name="prevProject"> The previous/latest version of the project. </param>
		/// <param name="unchangedFiles">
		/// The files that did not change in comparison with the previous version of the project.
		/// </param>
		void handleUpdateUnchangedFilesThreads(ProjectIn project, ProjectOut prevProject,
											   std::vector<File> unchangedFiles);

		/// <summary>
		/// Handles a single thread of updating methods in unchanged files.
		/// </summary>
		/// <param name="hashFiles"> A queue of pairs of hashes and fileLocations. </param>
		/// <param name="queueLock">
		/// The lock used for the queue 'hashFiles' in order to do multiple queries concurrently.
		/// </param>
		/// <param name="project"> The project corresponding to the hashes and fileLocations. </param>
		/// <param name="prevVersion"> The previous/latest version of the project. </param>
		/// <returns> The hashes in the queue 'hashFiles' that are in fact part of the unchanged files. </returns>
		std::vector<Hash>
		singleUpdateUnchangedFilesThread(std::queue<std::pair<std::vector<Hash>, std::vector<File>>> &hashFiles,
										 std::mutex &queueLock, ProjectIn project, long long prevVersion);

		/// <summary>
		/// Parses a list of authors with IDs to a string to be returned.
		/// </summary>
		/// <param name="authors"> A vector of pairs of authors and their corresponding ID. </param>
		/// <returns>
		/// A string of the format (where | and '\n' are defined as FIELD_DELIMITER_CHAR and 
		/// ENTRY_DELIMITER_CHAR respectively):
		/// name_1|mail_1|id_1'\n'name_2|mail_2|id_2'\n'...
		/// </returns>
		std::string authorsToString(std::vector<std::pair<Author, AuthorID>> authors);

		/// <summary>
		/// Parses a dataEntry to an author.
		/// </summary>
		/// <param name="dataEntry">
		/// A string with the name and mail of an author seperated by the FIELD_DELIMITER_CHAR.
		/// </param>
		/// <returns> An author with the name and mail passed to the method. </returns>
		Author datanEntryToAuthor(std::string dataEntry);

		/// <summary>
		/// Retrieves the authors corresponding to the IDs given as input using the database.
		/// </summary>
		/// <param name="authorIDs"> A vector of authorIDs. </param>
		/// <returns> A vector consisting of pairs of authors and their corresponding ID. </returns>
		std::vector<std::pair<Author, AuthorID>> getAuthors(std::vector<AuthorID> authorIDs);

		/// <summary>
		/// Handles a single thread of retrieving authors from the database.
		/// </summary>
		/// <param name="authorIDs"> The queue with author ids that have to be checked. </param>
		/// <param name="queueLock"> The lock for the queue with authorIDs. </param>
		/// <returns> A vector consisting of pairs with an author and the corresponding ID. </returns>
		std::vector<std::pair<Author, AuthorID>> singleIDToAuthorThread(std::queue<AuthorID> &authorIDs,
																		 std::mutex &queueLock);

		/// <summary>
		/// Retrieves the methods worked on by one of the authors for which the id is given.
		/// </summary>
		/// <param name="authorIDs"> A vector of authorIDs. </param
		/// <returns> All methods in the database that one of the give authors has worked on. </returns>
		std::vector<std::pair<MethodID, AuthorID>> getMethodsByAuthor(std::vector<AuthorID> authorIDs);

		/// <summary>
		/// Handles a single thread of retrieving methods by given authors.
		/// </summary>
		/// <param name="authorIDs"> The queue with IDs of authors that have to be checked. </param>
		/// <param name="queueLock"> The lock for the queue with authorIDs. </param>
		/// <returns> A vector consisting of pairs of methodIDs and authorIDs. </returns>
		std::vector<std::pair<MethodID, AuthorID>> singleAuthorToMethodsThread(std::queue<AuthorID> &authorIDs,
																				 std::mutex &queueLock);

		/// <summary>
		/// Parses a list of methods with authorIDs to a string to be returned.
		/// </summary>
		/// <param name="methods"> A vector of pairs containing a method and the corresponding authorID. </param>
		/// <returns>
		/// A string of the format (where | and '\n' are defined as FIELD_DELIMITER_CHAR and 
		/// ENTRY_DELIMITER_CHAR respectively):
		/// authorID_1|hash_1|projectID_1|version_1'\n'authorID_2|hash_2|projectID_2|version_2'\n'...
		/// <returns>
		std::string methodIDsToString(std::vector<std::pair<MethodID, AuthorID>> methods);

		/// <summary>
		/// Calls connect in the DatabaseHandler, if connect fails, it retries as many times as the MAX_RETRIES.
		/// If it still fails on the last retry, sets the errno to ENETUNREACH, so an exception can be thrown afterwards.
		/// </summary>
		/// <param name="ip"> The corresponding ip. </param>
		/// <param name="port"> The corresponding portnumber. </param>
		/// <returns> An empty tuple. </returns>
		std::tuple<> connectWithRetry(std::string ip, int port);

		/// <summary>
		/// Tries to upload a project to the database, if it fails it retries as many times as the MAX_RETRIES.
		/// If it succeeds, it returns true. If it still fails on the last retry, it returns false.
		/// </summary>
		/// <param name="project"> The corresponding project to be added. </param>
		bool tryUploadProjectWithRetry(ProjectIn project);

		/// <summary>
		/// Tries to add method to the database, if it fails it retries as many times as MAX_RETRIES.
		/// If it succeeds, it adds the method to the database. If it still fails on the last retry, 
		/// it puts the errno on ENETUNREACH.
		/// </summary>
		/// <param name="method"> The method to be added/updated. </param>
		/// <param name="project"> The project in which the method is located. </param>
		/// <param name="prevVersion"> The previous version of the project. </param>
		/// <param name="parserVersion"> the version of the parser. </param>
		/// <param name="newProject"> Indication of the project being new or not. </param>
		/// <returns> An empty tuple. </returns>
		std::tuple<> addMethodWithRetry(MethodIn method, ProjectIn project, long long prevVersion, long long parserVersion,
										bool newProject);

		/// <summary>
		/// Tries to obtain the previous/latest version of the relevant project.
		/// If it succeeds, either returns the project found, or returns an empty project with projectID = -1,
		/// if there is no such project inside the database. If it still fails on the last retry, it returns an empty project
		/// (with projectID = -1) and set the errno on ENETUNREACH.
		/// </summary>
		/// <param name="projectID"> The projectID of the project to be searched for. </param>
		/// <returns> The latest version of the project with the provided projectID. </returns>
		ProjectOut getPrevProjectWithRetry(ProjectID projectID);
	
		/// <summary>
		/// Tries to update the methods in the previous version of the project that are in an unchanged file.
		/// </summary>
		/// <param name="hashFile"> A pair of a hash and a file location. </param>
		/// <param name="project"> The corresponding updated version of the project. </param>
		/// <param name="prevVersion"> The previous versin of the project. </param>
		/// <returns>
		/// The hashes that correspond to methods that have been changed, used to add these hashes 
		/// to the project afterwards. If it fails to establish the hashes, puts the errno on ENETUNREACH 
		/// and returns empty vector.
		/// </returns>
		std::vector<Hash> updateUnchangedFilesWithRetry(std::pair<std::vector<Hash>, std::vector<File>> hashFile,
														ProjectIn project, long long prevVersion);

		/// <summary>
		/// Tries to get all methods with a given hash from the database, if it fails it retries as many times as 
		/// MAX_RETRIES. If it succeeds, it returns the methods found in the database.
		/// If it fails, it puts the errno on ENETUNREACH and returns an empty vector.
		/// </summary>
		/// <param name="hash"> The corresponding hash to be searched for. </param>
		/// <returns> The methods corresponding to the hash provided. </returns>
		std::vector<MethodOut> hashToMethodsWithRetry(Hash hash);

		/// <summary>
		/// Tries to get projects with a given version and projectID from the database, if it fails it retries as 
		/// many times as MAX_RETRIES. If it succeeds, it returns the projects. If it fails, it returns an empty
		/// vector and puts the errno on ENETUNREACH.
		/// </summary>
		/// <param name="projectID"> The corresponding projectID of the project to be searched for. </param>
		/// <param name="version"> The corresponding version of the project to be searched for. </param>
		/// <returns> The project corresponding to the key provided as input. </returns>
		ProjectOut searchForProjectWithRetry(ProjectID projectID, Version version);

		/// <summary>
		/// Tries to get author from the database given an authorID, if it fails it retries as many times as 
		/// MAX_RETRIES. If it succeeds, it returns the author. If it fails, it returns an emprt author and 
		/// puts errno on ENETUNREACH.
		/// </summary>
		Author idToAuthorWithRetry(AuthorID id);

		/// <summary>
		/// Tries to get methods from the database given an authorID, if it fails it retries like above.
		/// If it succeeds, it returns the methods.
		/// If it fails, it returns an empty vector and puts errno on ENETUNREACH.
		/// </summary>
		std::vector<MethodID> authorToMethodsWithRetry(AuthorID authorID);

		/// NEXT METHODS TO UTILITY IF TIME LEFT.

		/// <summary>
		/// Splits a list of arbitrary type into multiple chunks of size at most equal to the chunkSize.
		/// </summary>
		/// <param name="list">
		/// A list of elements of some type T that has to be split into multiple chunks.
		/// </param>
		/// <param name="chunkSize">
		/// The maximum size of a single chunk.
		/// </param>
		template <class T> std::vector<std::vector<T>> toChunks(std::vector<T> list, int chunkSize);

		/// <summary>
		/// Forms a queue of the elements in the cartesian product of two vectors of type T1 and T2 respectively.
		/// Here the cartesian product of two vectors K and L consists of all pairs <k, l> where k in K and l in L.
		/// </summary>
		/// <param name="listT1">
		/// A list of elements of type T1. It has the role of the container K.
		/// </param>
		/// <param name="listT2">
		/// A list of elements of type T2. It has the role of container L.
		/// </param>
		template <class T1, class T2>
		std::queue<std::pair<std::vector<T1>, std::vector<T2>>> cartesianProductQueue(std::vector<std::vector<T1>> listT1,
																					  std::vector<std::vector<T2>> listT2);

		DatabaseHandler *database;
};

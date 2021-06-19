/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include "Types.h"
#include <string>
#include <cassandra.h>

#define IP "cassandra"
#define DBPORT 8002
#define MAX_THREADS 32
#define HASHES_TO_INSERT_AT_ONCE 1000

using namespace types;

/// <summary>
/// Handles interaction with database.
/// </summary>
class DatabaseHandler
{
public:
	/// <summary>
	/// Connect to the database.
	/// </summary>
	virtual void connect(std::string ip, int port);

	/// <summary>
	/// Add a project to database. Takes a project as input and adds it to the database.
	/// </summary>
	virtual void addProject(ProjectIn project);

	/// <summary>
	/// Adds a number of hashes to a project in the database, starting at index.
	/// </summary>
	virtual void addHashToProject(ProjectIn project, int index);

	/// <summary>
	/// Searches for project in the database and returns them to the user. Takes primary key of project as input
	/// consisting of projectID and version. Returns the project corresponding to the input, if it exists.
	/// If no entry can be found, simply returns an empty vector.
	/// </summary>
	virtual ProjectOut searchForProject(ProjectID projectID, Version version);

	/// <summary>
	/// Retrieves the previous/latest version of the project present in the database.
	/// </summary>
	/// <returns>
	/// If present, returns the previous/latest version of a project with the same projectID.
	/// Else, sets the errno to ERANGE and returns an empty project.
	/// </returns>
	virtual ProjectOut prevProject(ProjectID projectID);

	/// <summary>
	/// Add/update a method to the tables methods and method_by_author. Takes in a method and a project and adds the method to
	/// the database with information of the project. 
	/// </summary>
	/// <param name="method">
	/// The method to be added/updated.
	/// </param>
	/// <param name="project">
	/// The project in which the method is located.
	/// </param>
	/// <param name="prevVersion">
	/// The previous version of the project.
	/// </param>
	/// <param name="parserVersion">
	/// The version of the parser.
	/// </param>
	/// <param name="newProject">
	/// A boolean value used to differentiate a new project from a changed project.
	/// The value is true if and only if the project is new.
	/// </param>
	virtual void addMethod(MethodIn method, ProjectIn project, long long prevVersion, long long parserVersion, bool newProject);

	/// <summary>
	/// Updates the methods in the previous version of the project that are in an unchanged file.
	/// </summary>
	/// <param name="hashes">
	/// A list of hashes to check.
	/// </param>
	/// <param name="files">
	/// A list of files to check.
	/// </param>
	/// <param name="project">
	/// The added project for the projectID and new version.
	/// </param>
	/// <param name="prevVersion">
	/// The previous version of the project to check whether it is a correct result.
	/// </param>
	/// <returns>
	/// A list of hashes that are updated.
	/// </returns>
	virtual std::vector<Hash> updateUnchangedFiles(std::vector<Hash> hashes, std::vector<std::string> files, ProjectIn project,
										   long long prevVersion);

	/// <summary>
	/// Given a hash, return all methods with that hash.
	/// </summary>
	/// <param name="hash">
	/// The hash to be checked.
	/// </param>
	/// <returns>
	/// All methods with the inputted hash.
	/// </returns>
	virtual std::vector<MethodOut> hashToMethods(std::string hash);

	/// <summary>
	/// Given an author id retrieves the corresponding author.
	/// </summary>
	/// <param name="id">
	/// A string with the id to be checked.
	/// </param>
	/// <returns>
	/// The author corresponding to the given id.
	/// </returns>
	virtual Author idToAuthor(std::string id);

	/// <summary>
	/// Given an author id retrieves the methods created by that author.
	/// </summary>
	/// <param name="authorId">
	/// The id of the author to retrieve the methods for.
	/// </param>
	/// <returns>
	/// A vector with the necessary information of the methods the author has worked on.
	/// </returns>
	virtual std::vector<MethodId> authorToMethods(std::string authorId);

private:
	/// <summary>
	/// Add a method to the method_by_author table.
	/// </summary>
	void addMethodByAuthor(CassUuid authorID, MethodIn method, ProjectIn project);

	/// <summary>
	/// Parses a row into a project. Takes a row as input and outputs a project.
	/// </summary>
	ProjectOut getProject(const CassRow *row);

	/// <summary>
	/// Parses a row into a method. Takes a row as input and outputs a method.
	/// </summary>
	MethodOut getMethod(const CassRow *row);

	/// <summary>
	/// Parses a row into a method id. Takes a row as input and outputs a method id.
	/// </summary>
	MethodId getMethodId(const CassRow *row);

	/// <summary>
	/// A function that is used to add new methods to the database.
	/// </summary>
	/// <param name="method">
	/// The method to be inputted into the database.
	/// </param>
	/// <param name="project">
	/// The project in which the method is located.
	/// </param>
	/// <param name="parserVersion">
	/// The version of the parser.
	/// </param>
	void addNewMethod(MethodIn method, ProjectIn project, long long parserVersion);

	/// <summary>
	/// A function that is used to update methods that were already in the database.
	/// More precisely, updates endVersionTime and -Hash, methodName, lineNumber and authors.
	/// </summary>
	/// <param name="method">
	/// The method to be updated.
	/// </param>
	/// <param name="project">
	/// The project in which the method is located.
	/// </param>
	/// <param name="startVersion">
	/// The startVersionTime of the method to be updated.
	/// </param>
	void updateMethod(MethodIn method, ProjectIn project, long long startVersion);

	/// <summary>
	/// Creates a new author and adds it to the database. Takes in the author to add.
	/// </summary>
	CassUuid createAuthorIfNotExists(Author author);

	/// <summary>
	/// Retrieves a string from a row. Takes in the row and the name of the column.
	/// </summary>
	std::string getString(const CassRow *row, const char *column);

	/// <summary>
	/// Retrieves a 32 bit integer from a row. Takes in the row and the name of the column.
	/// </summary>
	int getInt32(const CassRow *row, const char *column);

	/// <summary>
	/// Retrieves a 64 bit integer from a row. Takes in the row and the name of the column.
	/// </summary>
	long long getInt64(const CassRow *row, const char *column);

	/// <summary>
	/// Retrieves a UUID from a row and converts it into a string. Takes in the row and the name of the column.
	/// </summary>
	std::string getUUID(const CassRow *row, const char *column);

	/// <summary>
	/// The connection with the database.
	/// <summary>
	CassSession *connection;

	/// <summary>
	/// Create the prepared statements to be executed later.
	/// </summary>
	void setPreparedStatements();

	/// <summary>
	/// Prepares a specified statement (query) to be executed later.
	/// </summary>
	/// <param name="query">
	/// A string containing the CQL-query to be executed later.
	/// Question marks are used to indicate positions to be bound at a later stage.
	/// </param>
	/// <returns>
	/// The constant prepared statement that allows us to execute the query given as input.
	/// </returns>
	const CassPrepared *prepareStatement(std::string query);

	/// <summary>
	/// The prepared statements that can be executed.
	/// </summary>
	const CassPrepared *selectMethods;
	const CassPrepared *selectProject;
	const CassPrepared *selectPrevProject;
	const CassPrepared *insertProject;
	const CassPrepared *addHashesToProject;
	const CassPrepared *insertMethod;
	const CassPrepared *updateMethods;
	const CassPrepared *updateUnchangedMethods;
	const CassPrepared *selectMethod;
	const CassPrepared *selectUnchangedMethods;
	const CassPrepared *insertMethodByAuthor;
	const CassPrepared *selectMethodByAuthor;
	const CassPrepared *insertAuthorById;
	const CassPrepared *selectAuthorById;
};

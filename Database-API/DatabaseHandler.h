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
	virtual std::vector<ProjectOut> searchForProject(ProjectID projectID, Version version);

	/// <summary>
	/// Retrieves the previous version of the project present in the database.
	/// </summary>
	/// <returns>
	/// If present, returns the previous version of a project with the same projectID.
	/// Else, returns an empty project.
	/// </returns>
	virtual ProjectOut prevProject(ProjectID projectID, Version version);

	/// <summary>
	/// Add a method to the tables methods and method_by_author. Takes in a method and a project and adds the method to
	/// the database with information of the project. 
	/// </summary>
	/// <param name="changed">
	/// A boolean representing if the method is changed compared to the previous version of the project.
	/// </param>
	virtual void addMethod(MethodIn method, ProjectIn project, long long prevVersion);

	/// <summary>
	/// Retrieves the method from the method table in the database with the given hash, projectID, version and fileLocation, 
	/// if it exists. Otherwise returns an empty method.
	/// </summary>
	virtual MethodOut retrieveMethod(Hash hash, ProjectID projectID, Version version, std::string fileLocation);

	/// <summary>
	/// Given a hash, return all methods with that hash. Takes a hash as input and outputs a list of methods that match
	/// the hash.
	/// </summary>
	virtual std::vector<MethodOut> hashToMethods(std::string hash);

	/// <summary>
	/// Given an author returns the id of that author.
	/// </summary>
	/// <param name="author">
	/// The author to retrieve the id for.
	/// </param>
	/// <returns>
	/// A string representing the author id.
	/// </returns>
	virtual std::string authorToId(Author author);

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

	void addNewMethod(MethodIn method, ProjectIn project);

	void updateMethod(MethodIn method, ProjectIn project, long long startVersion);

	/// <summary>
	/// Retrieves the author ID corresponding to the given author.
	/// Also creates a new author if the author does not yet exist.
	/// <summary>
	CassUuid getAuthorId(Author author);

	/// <summary>
	/// Retrieves the author id corresponding to the given author.
	/// </summary>
	/// <param name="author">
	/// The author to retrieve the id for.
	/// </param>
	/// <returns>
	/// The id of the author.
	/// </returns>
	virtual CassUuid retrieveAuthorId(Author author);

	/// <summary>
	/// Creates a new author and adds it to the database. Takes in the author to add.
	/// </summary>
	CassUuid createAuthor(Author author);

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
	/// The prepared statements that can be executed.
	/// </summary>
	const CassPrepared *selectMethods;
	const CassPrepared *selectProject;
	const CassPrepared *selectPrevProject;
	const CassPrepared *insertProject;
	const CassPrepared *addHashesToProject;
	const CassPrepared *insertMethod;
	const CassPrepared *updateMethods;
	const CassPrepared *selectMethod;
	const CassPrepared *insertMethodByAuthor;
	const CassPrepared *selectMethodByAuthor;
	const CassPrepared *selectIdByAuthor;
	const CassPrepared *insertIdByAuthor;
	const CassPrepared *insertAuthorById;
	const CassPrepared *selectAuthorById;
};

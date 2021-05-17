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
	virtual void addProject(Project project);

	/// <summary>
	/// Add a method to the tables methods and method_by_author. Takes in a method and a project and adds the method to
	/// the database with information of the project.
	/// </summary>
	virtual void addMethod(MethodIn method, Project project);

	/// <summary>
	/// Given a hash, return all methods with that hash. Takes a hash as input and outputs a list of methods that match
	/// the hash.
	/// </summary>
	virtual std::vector<MethodOut> hashToMethods(std::string hash);

private:
	/// <summary>
	/// Add a method to the method_by_author table.
	/// </summary>
	void addMethodByAuthor(CassUuid authorID, MethodIn method, Project project);

	/// <summary>
	/// Parses a row into a method. Takes a row as input and outputs a method.
	/// </summary>
	MethodOut getMethod(const CassRow *row);

	/// <summary>
	/// Retrieves the author ID corresponding to the given author.
	/// <summary>
	CassUuid getAuthorID(Author author);

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
	const CassPrepared *selectMethod;
	const CassPrepared *insertProject;
	const CassPrepared *insertMethod;
	const CassPrepared *insertMethodByAuthor;
	const CassPrepared *selectIdByAuthor;
	const CassPrepared *insertIdByAuthor;
	const CassPrepared *insertAuthorById;
};

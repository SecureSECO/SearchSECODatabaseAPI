/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include "Types.h"
#include <string>
#include <cassandra.h>

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
	virtual void connect();
	
	/// <summary>
	/// Add a project to database. Takes a project as input and adds it to the database.
	/// </summary>
	virtual void addProject(Project project);

	/// <summary>
	/// Add a method to the tables methods and method_by_author. Takes in a method and a project and adds the method to the database with information of the project.
	/// </summary>
	virtual void addMethod(MethodIn method, Project project);

	/// <summary>
	/// Given a hash, return all methods with that hash. Takes a hash as input and outputs a list of methods that match the hash.
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
	MethodOut getMethod(const CassRow* row);

	/// <summary>
	/// Retrieves the author ID corresponding to the given author.
	/// <summary>
	CassUuid getAuthorID(Author author);

	/// <summary>
	/// The connection with the database.
	/// <summary>
	CassSession* connection;
};

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
	// Connect to the database
	virtual void connect();

	// Add a project to database.
	virtual void addProject(Project project);

	// Add a method to the tables methods and method_by_author.
	virtual void addMethod(MethodIn method, Project project);

	// Given a hash, return all methods with that hash.
	virtual std::vector<MethodOut> hashToMethods(std::string hash);
private:
	// Add a method to the method_by_author table.
	void addMethodByAuthor(CassUuid authorID, MethodIn method, Project project);

	// Parses a row into a method.
	MethodOut getMethod(const CassRow* row);

	// Retrieves the author ID corresponding to the given author.
	CassUuid getAuthorID(Author author);

	// The connection with the database.
	CassSession* connection;
};

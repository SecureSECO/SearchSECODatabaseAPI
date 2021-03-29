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
	void Connect();

	// Add a project to database.
	void AddProject(Project project);

	// Add a method to the tables methods and method_by_author.
	void AddMethod(MethodIn method, Project project);

	// Given a hash, return all methods with that hash
	std::vector<MethodOut> HashToMethods(std::string hash);
private:
	// Check if two methods are equivalent, i.e. contain the same hash.
	bool Equivalent(MethodIn method1, MethodIn method2);

	void AddMethodByAuthor(CassUuid authorID, MethodIn method, Project project);

	// Parses a row into a method.
	MethodOut GetMethod(const CassRow* row);

	CassUuid GetAuthorID(Author author);

	// The connection with the database.
	CassSession* connection;
};

/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include "Method.h"
#include "Project.h"

/// <summary>
/// Handles interaction with database.
/// </summary>
class DatabaseHandler
{
public:
	// Add a project to database.
	void AddProject(Project project);

	// Add a method to the tables methods and method_by_author.
	void AddMethod(Method method);

	// Given a hash, return all methods with that hash
	std::array<Method> HashToMethods(String hash);
private:
	// Check if two methods are equivalent, i.e. contain the same hash.
	bool Equivalent(Method method1, Method method2);
};
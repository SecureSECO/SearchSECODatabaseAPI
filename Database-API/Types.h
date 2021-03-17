/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include <ctime>
#include <string>

/// <summary>
/// Represents the data of an author.
/// </summary>
struct Author
{
public:
	std::string name;
	std::string mail;
};


/// <summary>
/// Represents the relevant data of a method.
/// </summary>
struct Method
{
public:
	std::string hash;
	std::string projectID;
	std::string methodName;
	std::string fileLocation;
	Author authors[];
};

/// <summary>
/// Represents the relevant data of a project.
/// </summary>
struct Project
{
public:
	std::string projectID;
	tm version;
	std::string license;
	std::string name;
	std::string url;
	Author owner;
	int stars;
	std::string hashes[];
};

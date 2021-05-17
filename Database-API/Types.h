/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include <ctime>
#include <string>
#include <vector>

namespace types {

typedef std::string AuthorID;
typedef long long ProjectID;
typedef std::string Hash;
typedef time_t Version;

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
/// Represents the relevant data of a method to be put in the database.
/// </summary>
struct MethodIn
{
public:
	Hash hash;
	std::string methodName;
	std::string fileLocation;
	int lineNumber;
	std::vector<Author> authors;
};

/// <summary>
/// Represents the data of a method to be returned to the user.
/// Difference with MethodIn: authorIDs instead of authors.
/// </summary>
struct MethodOut
{
public:
	Hash hash;
	ProjectID projectID;
	Version version;
	std::string methodName;
	std::string fileLocation;
	int lineNumber;
	std::vector<AuthorID> authorIDs;
};

struct MethodId
{
public:
	Hash hash;
	ProjectID projectId;
	Version version;
};

/// <summary>
/// Represents the relevant data of a project.
/// </summary>
struct Project
{
public:
	ProjectID projectID;
	Version version;
	std::string license;
	std::string name;
	std::string url;
	Author owner;
	int stars;
	std::vector<Hash> hashes;
};
}

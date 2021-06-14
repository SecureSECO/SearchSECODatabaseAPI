/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include <ctime>
#include <string>
#include <vector>

#include "md5/md5.h"

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
	std::string id;
	Author(std::string name, std::string mail) :
	name(name),
	mail(mail)
	{
		
		id = md5(name + " " + mail);
	}
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
/// Represents the relevant data of a project when it is put into the database.
/// </summary>
struct ProjectIn
{
public:
	ProjectID projectID;
	Version version;
	std::string license;
	std::string name;
	std::string url;
	Author owner = Author("","");
	std::vector<Hash> hashes;
};

/// <summary>
/// Represents the relevant data of a project when it is returned to user.
/// </summary>
struct ProjectOut
{
public:
	ProjectID projectID;
	Version version;
	std::string license;
	std::string name;
	std::string url;
	AuthorID ownerID;
	std::vector<Hash> hashes;
};
}

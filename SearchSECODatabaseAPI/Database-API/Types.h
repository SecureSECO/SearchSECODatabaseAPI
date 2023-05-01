/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include <ctime>
#include <vector>

#include "md5/md5.h"

namespace types
{

	typedef std::string AuthorID;
	typedef long long ProjectID;
	typedef std::string Hash;
	typedef std::string File;
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
		Author() : name(""), mail(""), id("")
		{
		}

		/// <summary>
		/// Constructs an author with provided name and mail,
		/// and generates id as the MD5-hash of the concatenated
		/// string of name and mail separated by ' '.
		/// </summary>
		Author(std::string name, std::string mail) : name(name), mail(mail)
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
		File fileLocation;
		int lineNumber;
		std::string vulnCode;
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
		File fileLocation;
		Version startVersion;
		std::string startVersionHash;
		Version endVersion;
		std::string endVersionHash;
		std::string methodName;
		int lineNumber;
		std::string vulnCode;
		std::vector<AuthorID> authorIDs;
		long long parserVersion;
		std::string license;
	};

	struct MethodID
	{
	  public:
		Hash hash;
		ProjectID projectID;
		File fileLocation;
		Version startVersion;
	};

	/// <summary>
	/// Represents the relevant data of a project when it is put into the database.
	/// </summary>
	struct ProjectIn
	{
	  public:
		ProjectID projectID;
		Version version;
		std::string versionHash;
		std::string license;
		std::string name;
		std::string url;
		Author owner = Author("", "");
		std::vector<Hash> hashes;
		long long parserVersion;
	};

	/// <summary>
	/// Represents the relevant data of a project when it is returned to user.
	/// Difference with ProjectIn: authorIDs instead of authors.
	/// </summary>
	struct ProjectOut
	{
	  public:
		ProjectID projectID;
		Version version;
		std::string versionHash;
		std::string license;
		std::string name;
		std::string url;
		AuthorID ownerID;
		std::vector<Hash> hashes;
		long long parserVersion;
	};
} // namespace types

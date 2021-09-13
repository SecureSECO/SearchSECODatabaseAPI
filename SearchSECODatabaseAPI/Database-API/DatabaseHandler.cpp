/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "DatabaseHandler.h"
#include "DatabaseUtility.h"

#include <iostream>
#include <unistd.h>

void DatabaseHandler::connect(std::string ip, int port)
{
	connection = DatabaseUtility::connect(ip, port, "projectdata");
	setPreparedStatements();
}

void DatabaseHandler::setPreparedStatements()
{
	// Prepare query used to select all methods with a given hash.
	selectMethods = prepareStatement("SELECT * FROM projectdata.methods WHERE method_hash = ?");

	// Prepare query used to select all projects with a given projectID and version(time).
	selectProject = prepareStatement("SELECT * FROM projectdata.projects WHERE projectID = ? AND versiontime = ?");
	
	// Prepare query used to select the previous/latest version with the given projectID.
	selectPrevProject = prepareStatement("SELECT * FROM projectdata.projects WHERE projectID = ? LIMIT 1");

	// Prepare query used to insert a project into the database.
	insertProject = prepareStatement("INSERT INTO projectdata.projects (projectID, versiontime, versionhash, license, "
									 "name, url, ownerID, hashes, parserversion) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");

	// Prepare query used to add hashes to a project in the database.
	addHashesToProject =
		prepareStatement("UPDATE projectdata.projects SET hashes = hashes + ? WHERE projectID = ? AND versiontime = ?");

	// Prepare query used to insert a method into the database.
	insertMethod = prepareStatement("INSERT INTO projectdata.methods (method_hash, projectID, startversiontime, file, "
									"startversionhash, endversiontime, endversionhash, name, lineNumber, authors, "
									"parserversion) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

	// Prepare query used to update a method in the database.
	updateMethods = prepareStatement("UPDATE projectdata.methods SET endVersionTime = ?, endVersionHash = ?, name = ?, "
									 "lineNumber = ?, authors = authors + ? WHERE method_hash = ? AND projectID = ? "
									 "AND file = ? AND startVersionTime = ?");

	// Prepare query used to select a method from the database given a hash, projectID and a file location.
	selectMethod =
		prepareStatement("SELECT * FROM projectdata.methods WHERE method_hash = ? AND projectID = ? AND file = ?");

	// Prepare query used to select the unchanged methods based on hash, projectID and file location.
	selectUnchangedMethods = prepareStatement(
		"SELECT method_hash, projectid, file, startversiontime, endversiontime, linenumber, name, startversionhash, "
		"endversionhash FROM projectdata.methods WHERE method_hash IN ? AND projectID = ? AND file IN ?");

	// Prepare query used to relate an author to a certain method (inside the method_by_author table).
	insertMethodByAuthor = prepareStatement("INSERT INTO projectdata.method_by_author (authorID, hash, "
											"startversiontime, file, projectID) VALUES (?, ?, ?, ?, ?)");

	// Prepare query used to select a method given the ID of an author (by means of the method_by_author table).
	selectMethodByAuthor = prepareStatement("SELECT * FROM projectdata.method_by_author WHERE authorid = ?");

	// Prepare query used to relate an author to an ID of the author (inside the author_by_id table).
	insertAuthorByID =
		prepareStatement("INSERT INTO projectdata.author_by_id (authorID, name, mail) VALUES (?, ?, ?) IF NOT EXISTS");

	// Prepare query used to select an author given its ID (bby means of the author_by_id table).
	selectAuthorByID = prepareStatement("SELECT * FROM projectdata.author_by_id WHERE authorid = ?");
}

const CassPrepared* DatabaseHandler::prepareStatement(std::string query)
{
	CassFuture *prepareFuture =	cass_session_prepare(connection, query.c_str());
	CassError rc = cass_future_error_code(prepareFuture);
	if (rc != 0)
	{
		// An error occurred which is handled below.
		const char *message;
		size_t messageLength;
		cass_future_error_message(prepareFuture, &message, &messageLength);
		fprintf(stderr, "Unable to prepare query: '%.*s'\n", (int)messageLength, message);
	}

	const CassPrepared *result = cass_future_get_prepared(prepareFuture);

	cass_future_free(prepareFuture);

	return result;
}

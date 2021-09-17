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
	selectMethods =
		DatabaseUtility::prepareStatement(connection, "SELECT * FROM projectdata.methods WHERE method_hash = ?");

	// Prepare query used to select all projects with a given projectID and version(time).
	selectProject = DatabaseUtility::prepareStatement(
		connection, "SELECT * FROM projectdata.projects WHERE projectID = ? AND versiontime = ?");

	// Prepare query used to select the previous/latest version with the given projectID.
	selectPrevProject =
		DatabaseUtility::prepareStatement(connection, "SELECT * FROM projectdata.projects WHERE projectID = ? LIMIT 1");

	// Prepare query used to insert a project into the database.
	insertProject = DatabaseUtility::prepareStatement(
		connection, "INSERT INTO projectdata.projects (projectID, versiontime, versionhash, license, "
					"name, url, ownerID, hashes, parserversion) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");

	// Prepare query used to add hashes to a project in the database.
	addHashesToProject = DatabaseUtility::prepareStatement(
		connection, "UPDATE projectdata.projects SET hashes = hashes + ? WHERE projectID = ? AND versiontime = ?");

	// Prepare query used to insert a method into the database.
	insertMethod = DatabaseUtility::prepareStatement(
		connection, "INSERT INTO projectdata.methods (method_hash, projectID, startversiontime, file, "
					"startversionhash, endversiontime, endversionhash, name, lineNumber, authors, "
					"parserversion) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

	// Prepare query used to update a method in the database.
	updateMethods = DatabaseUtility::prepareStatement(
		connection, "UPDATE projectdata.methods SET endVersionTime = ?, endVersionHash = ?, name = ?, "
					"lineNumber = ?, authors = authors + ? WHERE method_hash = ? AND projectID = ? "
					"AND file = ? AND startVersionTime = ?");

	// Prepare query used to select a method from the database given a hash, projectID and a file location.
	selectMethod = DatabaseUtility::prepareStatement(
		connection, "SELECT * FROM projectdata.methods WHERE method_hash = ? AND projectID = ? AND file = ?");

	// Prepare query used to select the unchanged methods based on hash, projectID and file location.
	selectUnchangedMethods = DatabaseUtility::prepareStatement(
		connection,
		"SELECT method_hash, projectid, file, startversiontime, endversiontime, linenumber, name, startversionhash, "
		"endversionhash FROM projectdata.methods WHERE method_hash IN ? AND projectID = ? AND file IN ?");

	// Prepare query used to relate an author to a certain method (inside the method_by_author table).
	insertMethodByAuthor =
		DatabaseUtility::prepareStatement(connection, "INSERT INTO projectdata.method_by_author (authorID, hash, "
													  "startversiontime, file, projectID) VALUES (?, ?, ?, ?, ?)");

	// Prepare query used to select a method given the ID of an author (by means of the method_by_author table).
	selectMethodByAuthor =
		DatabaseUtility::prepareStatement(connection, "SELECT * FROM projectdata.method_by_author WHERE authorid = ?");

	// Prepare query used to relate an author to an ID of the author (inside the author_by_id table).
	insertAuthorByID = DatabaseUtility::prepareStatement(
		connection, "INSERT INTO projectdata.author_by_id (authorID, name, mail) VALUES (?, ?, ?) IF NOT EXISTS");

	// Prepare query used to select an author given its ID (bby means of the author_by_id table).
	selectAuthorByID =
		DatabaseUtility::prepareStatement(connection, "SELECT * FROM projectdata.author_by_id WHERE authorid = ?");
}
/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "DatabaseHandler.h"

#include <iostream>
#include <unistd.h>

void DatabaseHandler::connect(std::string ip, int port)
{
	errno = 0;
	CassFuture *connectFuture = NULL;
	CassCluster *cluster = cass_cluster_new();
	connection = cass_session_new();

	// Add contact points.
	cass_cluster_set_contact_points(cluster, ip.c_str());
	cass_cluster_set_port(cluster, port);
	cass_cluster_set_protocol_version(cluster, CASS_PROTOCOL_VERSION_V3);
	cass_cluster_set_consistency(cluster, CASS_CONSISTENCY_QUORUM);
	cass_cluster_set_num_threads_io(cluster, MAX_THREADS);

	std::cout << "Connecting to the database." << std::endl;

	// Provide the cluster object as configuration to connect the session.
	connectFuture = cass_session_connect_keyspace(connection, cluster, "projectdata");

	CassError rc = cass_future_error_code(connectFuture);

	if (rc != CASS_OK)
	{
		std::cout << "Could not connect to the database." << std::endl;
		std::cout << "Retrying after 45 seconds have elapsed.." << std::endl;
		usleep(45000000);
		std::cout << "Retrying now." << std::endl;

		connectFuture = cass_session_connect_keyspace(connection, cluster, "projectdata");

		CassError rc = cass_future_error_code(connectFuture);

		if (rc != CASS_OK)
		{
			// An error occurred, display connection error message.
			const char *message;
			size_t messageLength;
			cass_future_error_message(connectFuture, &message, &messageLength);
			fprintf(stderr, "Connect error: '%.*s'\n", (int)messageLength, message);
			errno = ENETUNREACH;
		}
	}
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

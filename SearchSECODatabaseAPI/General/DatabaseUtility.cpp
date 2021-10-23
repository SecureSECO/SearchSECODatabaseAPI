/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "DatabaseUtility.h"

#include <iostream>
#include <unistd.h>

CassSession *DatabaseUtility::connect(std::string ip, int port, std::string keyspace)
{
	errno = 0;
	CassFuture *connectFuture = NULL;
	CassCluster *cluster = cass_cluster_new();
	CassSession *connection = cass_session_new();

	// Add contact points.
	cass_cluster_set_contact_points(cluster, ip.c_str());
	cass_cluster_set_port(cluster, port);
	cass_cluster_set_protocol_version(cluster, CASS_PROTOCOL_VERSION_V3);
	cass_cluster_set_consistency(cluster, CASS_CONSISTENCY_QUORUM);
	cass_cluster_set_num_threads_io(cluster, MAX_THREADS);

	std::cout << "Connecting to the database. With keyspace: " << keyspace << std::endl;

	// Provide the cluster object as configuration to connect the session.
	connectFuture = cass_session_connect_keyspace(connection, cluster, keyspace.c_str());

	CassError rc = cass_future_error_code(connectFuture);

	if (rc != CASS_OK)
	{
		std::cout << "Could not connect to the database." << std::endl;
		std::cout << "Retrying after 45 seconds have elapsed.." << std::endl;
		usleep(45000000);
		std::cout << "Retrying now." << std::endl;

		connectFuture = cass_session_connect_keyspace(connection, cluster, keyspace.c_str());

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
	return connection;
}

const CassPrepared *DatabaseUtility::prepareStatement(CassSession *connection, std::string query)
{
	CassFuture *prepareFuture = cass_session_prepare(connection, query.c_str());
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

std::string DatabaseUtility::getString(const CassRow *row, const char *column)
{
	const char *result;
	size_t len;
	const CassValue *value = cass_row_get_column_by_name(row, column);
	cass_value_get_string(value, &result, &len);
	return std::string(result, len);
}

int DatabaseUtility::getInt32(const CassRow *row, const char *column)
{
	cass_int32_t result;
	const CassValue *value = cass_row_get_column_by_name(row, column);
	cass_value_get_int32(value, &result);
	return result;
}

long long DatabaseUtility::getInt64(const CassRow *row, const char *column)
{
	cass_int64_t result;
	const CassValue *value = cass_row_get_column_by_name(row, column);
	cass_value_get_int64(value, &result);
	return result;
}

std::string DatabaseUtility::getUUID(const CassRow *row, const char *column)
{
	char result[CASS_UUID_STRING_LENGTH];
	CassUuid authorID;
	const CassValue *value = cass_row_get_column_by_name(row, column);
	cass_value_get_uuid(value, &authorID);
	cass_uuid_string(authorID, result);
	return result;
}
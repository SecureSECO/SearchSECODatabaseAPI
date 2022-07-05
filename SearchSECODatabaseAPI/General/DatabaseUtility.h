/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include "Definitions.h"
#include <cassandra.h>
#include <string>

/// <summary>
/// Implements generic database functionality.
/// </summary>
class DatabaseUtility
{
public:
	/// <summary>
	/// Establishes a connection to the database.
	/// </summary>
	/// <param name="connection"> Pointer to the connection variable to write to. </param>
	/// <param name="ip"> The ip in string format. </param>
	/// <param name="port"> The portnumber to connect to. </param>
	/// <param name="keyspace"> The keyspace to connect to. </param>
	static CassSession *connect(std::string ip, int port, std::string keyspace);

	/// <summary>
	/// Prepares a specified statement (query) to be executed later.
	/// </summary>
	/// <param name="connection"> The connection to prepare the statement on. </param>
	/// <param name="query"> A string containing the CQL-query to be executed later. </param>
	/// <returns> The constant prepared statement that allows us to execute the query given as input. </returns>
	static const CassPrepared *prepareStatement(CassSession *connection, std::string query);

	/// <summary>
	/// Retrieves a string in a column and some row.
	/// </summary>
	/// <param name="row"> The corresponding row. </param>
	/// <param name="column"> The corresponding column. </param>
	static std::string getString(const CassRow *row, const char *column);

	/// <summary>
	/// Retrieves a 32-bit integer from a row.
	/// </summary>
	/// <param name="row"> The corresponding row. </param>
	/// <param name="column"> The corresponding column. </param>
	static int getInt32(const CassRow *row, const char *column);

	/// <summary>
	/// Retrieves a 64-bit integer from a row.
	/// </summary>
	/// <param name="row"> The corresponding row. </param>
	/// <param name="column"> The corresponding column. </param>
	static long long getInt64(const CassRow *row, const char *column);

	/// <summary>
	/// Retrieves a UUID from a row and converts it to a string.
	/// </summary>
	/// <param name="row"> The corresponding row. </param>
	/// <param name="column"> The corresponding column. </param>
	static std::string getUUID(const CassRow *row, const char *column);
};
﻿/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include "Types.h"

#include <tuple>
#include <cassandra.h>

#define IP "cassandra"
#define DBPORT 8002
#define HASHES_INSERT_MAX 1000

using namespace types;

/// <summary>
/// Handles interaction with database.
/// </summary>
class DatabaseHandler
{
public:
	/// <summary>
	/// Establishes a connection to the database.
	/// </summary>
	/// <param name="ip"> The ip in string format. </param>
	/// <param name="port"> The portnumber to connect to. </param>
	virtual void connect(std::string ip, int port);

	/// <summary>
	/// Adds a project to database.
	/// </summary>
	/// <param name="project"> The project to be added to the database. </param>
	/// <returns> A boolean value indicating if adding the project was successful. </returns>
	virtual bool addProject(ProjectIn project);

	/// <summary>
	/// Adds a number of hashes to a project in the database, starting at some index.
	/// </summary>
	/// <param name="project"> The project from which the hashes should be added to the database. </param>
	/// <param name="index"> The starting index of the hashes which should be added to the database. </param>
	virtual void addHashToProject(ProjectIn project, int index);

	/// <summary>
	/// Searches for a project in the database and returns it to the user if it exists.
	/// </summary>
	/// <param name="projectID"> The projectID of the project to be searched for. </param>
	/// <param name="version"> The version of the project to be searched for. </param>
	/// <returns>
	/// Returns the project corresponding to the input, if it exists.
	/// If no entry can be found, simply returns an empty project and sets the errno to ERANGE.
	/// </returns>
	virtual ProjectOut searchForProject(ProjectID projectID, Version version);

	/// <summary>
	/// Retrieves the previous/latest version of the project present in the database.
	/// </summary>
	/// <param name="projectID"> The projectID of the project to be searched for. </param>
	/// <returns>
	/// If present, returns the previous/latest version of a project with the same projectID.
	/// Else, sets the errno to ERANGE and returns an empty project.
	/// </returns>
	virtual ProjectOut prevProject(ProjectID projectID);

	/// <summary>
	/// Adds/updates a method to the tables methods and method_by_author. Takes in a method and a project
	/// and adds the method to the database with specific information of the project.
	/// </summary>
	/// <param name="method"> The method to be added/updated. </param>
	/// <param name="project"> The project in which the method is located. </param>
	/// <param name="prevVersion"> The previous version of the project. </param>
	/// <param name="parserVersion"> The version of the parser. </param>
	/// <param name="newProject"> Indication of the project being new or updated based on a previous version. </param>
	virtual void addMethod(MethodIn method, ProjectIn project, long long prevVersion, long long parserVersion,
						   bool newProject);

	/// <summary>
	/// Updates the methods in the previous version of the project that are part of an unchanged file.
	/// </summary>
	/// <param name="hashes"> A list of hashes to be checked. </param>
	/// <param name="files"> A list of files to be checked. </param>
	/// <param name="project"> The added project for the projectID and new version. </param>
	/// <param name="prevVersion"> The previous version of the project to check whether it is a correct result. </param>
	/// <returns> A list of hashes that are updated. </returns>
	virtual std::vector<Hash> updateUnchangedFiles(std::vector<Hash> hashes, std::vector<std::string> files,
												   ProjectIn project, long long prevVersion);

	/// <summary>
	/// Retrieves all methods with a given hash.
	/// </summary>
	/// <param name="hash"> The hash to be checked. </param>
	/// <returns> All methods with the inputted hash. </returns>
	virtual std::vector<MethodOut> hashToMethods(std::string hash);

	/// <summary>
	/// Retrieves an author given its authorID.
	/// </summary>
	/// <param name="id"> A string with the ID to be checked. </param>
	/// <returns> The author corresponding to the given ID. </returns>
	virtual Author idToAuthor(std::string id);

	/// <summary>
	/// Retrieves the methods created by an author given its authorID.
	/// </summary>
	/// <param name="authorID"> The ID of the author to retrieve the methods for. </param>
	/// <returns> A vector with the necessary information of the methods the author has worked on. </returns>
	virtual std::vector<MethodID> authorToMethods(std::string authorID);

private:
	/// <summary>
	/// Add a method to the method_by_author table.
	/// </summary>
	void addMethodByAuthor(CassUuid authorID, MethodIn method, ProjectIn project);

	/// <summary>
	/// Parses a row into a project. Takes a row as input and outputs a project.
	/// </summary>
	ProjectOut getProject(const CassRow *row);

	/// <summary>
	/// Parses a row into a method. Takes a row as input and outputs a method.
	/// </summary>
	MethodOut getMethod(const CassRow *row);

	/// <summary>
	/// Parses a row into a method id. Takes a row as input and outputs a method id.
	/// </summary>
	MethodID getMethodID(const CassRow *row);

	/// <summary>
	/// A function that is used to add new methods to the database.
	/// </summary>
	/// <param name="method"> The method to be inputted into the database. </param>
	/// <param name="project"> The project in which the method is located. </param>
	/// <param name="parserVersion"> The version of the parser. </param>
	void addNewMethod(MethodIn method, ProjectIn project, long long parserVersion);

	/// <summary>
	/// A function that is used to update methods that were already in the database.
	/// More precisely, updates endVersionTime and -Hash, methodName, lineNumber and authors.
	/// </summary>
	/// <param name="method"> The method to be updated. </param>
	/// <param name="project"> The project in which the method is located. </param>
	/// <param name="startVersion"> The startVersionTime of the method to be updated. </param>
	void updateMethod(MethodIn method, ProjectIn project, long long startVersion);

	/// <summary>
	/// Handles the result obtained by performing the select method query.
	/// </summary>
	/// <param name="queryFuture"> The query that has been performed. </param>
	/// <param name="method"> The method to has to be added/updated. </param>
	/// <param name="project"> The project in which the method is located. </param>
	/// <param name="prevVersion"> The previous version of the project. </param>
	/// <param name="parserVersion"> The version of the parser. </param>
	/// <param name="newProject"> Indication if the project is new or not. </param>
	void handleSelectMethodQueryResult(CassFuture *queryFuture, MethodIn method, ProjectIn project,
									   long long prevVersion, long long parserVersion, bool &newProject);

	/// <summary>
	/// Handles the result obtained by performing the select unchanged methods query.
	/// </summary>
	/// <param name="queryFuture"> The query that has been performed. </param>
	/// <param name="project"> The updated version of a project that has to be added. </param>
	/// <param name="prevVersion"> The previous version of the project. </param>
	/// <returns> Returns the hashes corresponding to unchanged methods. </returns>
	std::vector<Hash> handleSelectUnchangedMethodsResult(CassFuture *queryFuture, ProjectIn project,
														 long long prevVersion);

	/// <summary>
	/// Executes the query performed to select the unchanged methods.
	/// </summary>
	/// <param name="hashes"> The hashes to be checked for to select unchanged methods. </param>
	/// <param name="files"> The files to be checked for to select unchanged methods. </param>
	/// <param name="project"> The updated version of a project. </param>
	/// <returns> An executed query. </returns>
	CassFuture *executeSelectUnchangedMethodsQuery(std::vector<Hash> hashes, std::vector<std::string> files,
												   ProjectIn project);
	/// <summary>
	/// Creates a new author and adds it to the database.
	/// </summary>
	/// <param name="author"> Author to be added to the database (if it does not exist already). </param>
	/// <returns> The corresponding UUID. </returns>
	CassUuid createAuthorIfNotExists(Author author);

	/// <summary>
	/// Create the prepared statements to be executed later.
	/// </summary>
	void setPreparedStatements();

	/// <summary>
	/// The connection with the database.
	/// <summary>
	CassSession *connection;

	/// <summary>
	/// The prepared statements that can be executed after preparation.
	/// </summary>
	const CassPrepared *selectMethods;
	const CassPrepared *selectProject;
	const CassPrepared *selectPrevProject;
	const CassPrepared *insertProject;
	const CassPrepared *addHashesToProject;
	const CassPrepared *insertMethod;
	const CassPrepared *updateMethods;
	const CassPrepared *updateUnchangedMethods;
	const CassPrepared *selectMethod;
	const CassPrepared *selectUnchangedMethods;
	const CassPrepared *insertMethodByAuthor;
	const CassPrepared *selectMethodByAuthor;
	const CassPrepared *insertAuthorByID;
	const CassPrepared *selectAuthorByID;
};
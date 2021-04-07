/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include "DatabaseHandler.h"
#include <json.hpp>

enum eRequestType
{
    eUpload,
    eCheck,
    eCheckUpload,
    eUnknown
};



/// <summary>
/// Handles requests towards database.
/// </summary>
class RequestHandler
{
public:
	void initialize(DatabaseHandler databaseHandler);
	std::string handleRequest(std::string requestType, std::string request);
private:
	DatabaseHandler database;
	std::string handleUploadRequest(std::string request);
	Project requestToProject(std::string request);
	MethodIn dataEntryToMethod(std::string dataEntry);
	std::vector<Hash> requestToHashes(std::string request);
	std::string methodsToString(std::vector<MethodOut> methods, char dataDelimiter, char methodDelimiter);
	std::vector<MethodOut> getMethods(std::vector<Hash> hashes);
	void appendBy(std::vector<char>& result, std::string word, char delimiter);
	std::vector<std::string> splitStringOn(std::string str, char delimiter);
	std::string handleCheckRequest(std::string request);
	std::string handleCheckRequest(std::vector<Hash> hashes);
	std::string handleCheckUploadRequest(std::string request);
	std::string handleUnknownRequest();
	eRequestType getERequestType(std::string requestType);
};

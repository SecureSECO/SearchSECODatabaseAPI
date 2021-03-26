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
    eAddProject,
    eQuery,
    eUnknown
};



/// <summary>
/// Handles requests towards database.
/// </summary>
class RequestHandler
{
public:
	void Initialize();
	std::string HandleRequest(std::string requestType, std::string request);
private:
	DatabaseHandler database;
	void HandleAddProjectRequest(std::string request);
	Project RequestToProject(std::string request);
	MethodIn DataEntryToMethod(std::string dataEntry);
	std::vector<Hash> RequestToHashes(std::string request);
	std::string MethodsToString(std::vector<MethodOut> methods, char dataDelimiter, char methodDelimiter);
	std::vector<MethodOut> GetMethods(std::vector<Hash> hashes);
	void AppendBy(std::vector<char>& result, std::string word, char delimiter);
	std::vector<std::string> SplitStringOn(std::string str, char delimiter);
	std::string HandleQueryRequest(std::string request);
	void HandleUnknownRequest();
	eRequestType GetERequestType(std::string requestType);
};

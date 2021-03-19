/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include "DatabaseHandler.h"


enum eRequestType
{
    eAddProject,
    eAddMethod,
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
	std::string HandleRequest(std::string request);
private:
	DatabaseHandler database;
	void HandleAddProjectRequest(std::string request);
	void HandleAddMethodRequest(std::string request);
	std::string HandleQueryRequest(std::string request);
	void HandleUnknownRequest();
	Project JsonToProject(std::string request);
	std::tuple <MethodIn, ProjectID, Version> JsonToTuple(std::string request);
	std::string ToString(std::vector<std::string> values);
	eRequestType RequestToRequestType(std::string request);
};

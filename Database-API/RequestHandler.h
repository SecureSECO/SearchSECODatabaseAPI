/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
/// <summary>
/// Handles requests towards database.
/// </summary>
class RequestHandler
{
public:
	static void Initialise();
	static void HandleRequests(DatabaseHandler db);
	static void HandleAddProjectRequest(DatabaseHandler db, std::string request);
	static void HandleAddMethodRequest(DatabaseHandler db, std::string request);
	static void HandleQueryRequest(DatabaseHandler db, std::string request);
	static void HandleUnknownRequest();
};
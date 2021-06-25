/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include <string>

namespace HTTPStatusCodes
{
	/// <summary>
	/// Adds a small part at the beginning of the message to
	/// indicate that the action performed was successful.
	/// </summary>
	std::string success(std::string message);

	/// <summary>
	/// Adds a small part at the beginning of the message to
	/// indicating an error on the client-side.
	/// </summary>
	std::string clientError(std::string message);

	/// <summary>
	/// Adds a small part at the beginning of the message to
	/// indicating an error on the server-side.
	/// </summary>
	std::string serverError(std::string message);

	/// <summary>
	/// Obtains the code to glue to the message.
	/// </summary>
	std::string getCode(std::string request);

	/// <summary>
	/// Removes the code which was glued to the message.
	/// </summary>
	std::string getMessage(std::string request);
}

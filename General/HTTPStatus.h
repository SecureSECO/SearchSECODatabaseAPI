/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once

#include "Definitions.h"

#include <string>

namespace HTTPStatusCodes
{
	std::string success(std::string message);
	std::string clientError(std::string message);
	std::string getCode(std::string request);
	std::string getMessage(std::string request);
}

/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once

/// <summary>
/// Represents the relevant data of a method.
/// </summary>
struct Method
{
public:
	String hash;
	String projectID;
	String methodName;
	std::array<Author> authors;
	String fileLocation;
};


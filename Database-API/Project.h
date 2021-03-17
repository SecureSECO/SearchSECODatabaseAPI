/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include <ctime>

/// <summary>
/// Represents the relevant data of a project.
/// </summary>
struct Project
{
public:
	String projectID;
	tm version;
	String license;
	String name;
	String url;
	Author owner;
	int stars;
	std::array<String> hashes;
};
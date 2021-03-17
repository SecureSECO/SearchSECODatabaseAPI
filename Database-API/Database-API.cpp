/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include <iostream>
#include "Types.h"
#include "Database-API.h"
#include "DatabaseHandler.h"
#include "ConnectionHandler.h"

using namespace std;

int main()
{
	DatabaseHandler database;
	database.Connect();

	/*Project project;
	project.projectID = "de485100-2a23-4373-a308-b33378a6e6be";
	project.version = time(0);
	project.license = "Free";
	project.name = "Test";
	project.url = "test.test";
	Author owner;
	owner.name = "Mister owner";
	owner.mail = "owner@test.test";
	project.owner = owner;
	project.stars = 69;
	vector<string> hashes;
	hashes.push_back("de485100-2a23-4373-a308-b33378a6e6be");
	project.hashes = hashes;

	Method method;
	method.hash = "de485100-2a23-4373-a308-b33378a6e6be";
	method.methodName = "NiceTestMethod";
	method.fileLocation = "Great/File/Location.c";
	vector<Author> authors;
	authors.push_back(owner);
	method.authors = authors;

	cout << "Adding project" << endl;*/

	//vector<Method> methods = database.HashToMethods("de485100-2a23-4373-a308-b33378a6e6be");

	ConnectionHandler listen;
	listen.StartListen();

	//cout << methods[0].hash << endl;

	cout << "Hello CMake." << endl;
	return 0;
}

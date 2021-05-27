/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "RequestHandler.h"
#include "Utility.h"
#include "JDDatabaseMock.cpp"
#include "RaftConsensusMock.cpp"
#include "DatabaseMock.cpp"
#include <gtest/gtest.h>
#include <boost/shared_ptr.hpp>

// Checks if extract projects request works correctly when the request is empty.
TEST(ExtractProjectsRequestTests, Empty)
{
	RequestHandler handler;

	std::string input1 = "";
	std::string expected1 = "No results found.";

	std::string output1 = handler.handleRequest("extp", input1, nullptr);
	ASSERT_EQ(output1, expected1);
}

// Checks if the extract projects request works correctly when searching for a single existing project.
TEST(ExtractProjectsRequestTests, SingleExistingProject)
{
	MockDatabase database;
	RequestHandler handler;
	MockRaftConsensus raftConsensus;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	ProjectID projectID2 = 1;
	Version version2 = 5000000000000;
	std::string input2 = "1?5000000000000";
	ProjectOut project2 = {.projectID = 1,
						   .version = 5000000000000,
						   .license = "L1",
						   .name = "P1",
						   .url = "www.github.com/p1",
						   .ownerID = "68bd2db6-fe91-47d2-a134-cf82b104f547",
						   .hashes = {"2c7f46d4f57cf9e66b03213358c7ddb5"}};
	std::vector<ProjectOut> p2 = {project2};
	std::string expected2 = "1?5000000000000?L1?P1?www.github.com/p1?"
							"68bd2db6-fe91-47d2-a134-cf82b104f547\n";

	EXPECT_CALL(database, searchForProject(projectID2, version2)).WillOnce(testing::Return(p2));
	std::string output2 = handler.handleRequest("extp", input2, nullptr);
	ASSERT_EQ(output2, expected2);
}

// Checks if the extract projects request works correctly when searching for a non-existing project.
TEST(ExtractProjectsRequestTests, SingleNonExistingProject)
{
	MockDatabase database;
	RequestHandler handler;
	MockRaftConsensus raftConsensus;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	ProjectID projectID3 = 1;
	Version version3 = 5000000001000;
	std::string input3 = "1?5000000001000";
	std::vector<ProjectOut> p3 = {};
	std::string expected3 = "No results found.";

	EXPECT_CALL(database, searchForProject(projectID3, version3)).WillOnce(testing::Return(p3));
	std::string output3 = handler.handleRequest("extp", input3, nullptr);
	ASSERT_EQ(output3, expected3);
}

// Checks if the extract projects request works correctly when searching for multiple projects.
TEST(ExtractProjectsRequestTests, MultipleProjects)
{
	MockDatabase database;
	RequestHandler handler;
	MockRaftConsensus raftConsensus;
	MockJDDatabase jddatabase;
	handler.initialize(&database, &jddatabase, &raftConsensus);

	ProjectID projectID4_1 = 2;
	Version version4_1 = 5000000001000;
	ProjectID projectID4_2 = 2;
	Version version4_2 = 5000000002000;
	ProjectID projectID4_3 = 3;
	Version version4_3 = 5000000002000;
	ProjectID projectID4_4 = 4;
	Version version4_4 = 5000000005000;
	std::string input4 = "2?5000000001000\n2?5000000002000\n3?5000000002000\n4?5000000005000";
	ProjectOut project4_1 = {.projectID = 2,
							 .version = 5000000001000,
							 .license = "L2",
							 .name = "P2",
							 .url = "www.github.com/p2",
							 .ownerID = "68bd2db6-fe91-47d2-a134-cf82b104f547",
							 .hashes = {"06f73d7ab46184c55bf4742b9428a4c0"}};
	ProjectOut project4_2 = {.projectID = 3,
							 .version = 5000000002000,
							 .license = "L3",
							 .name = "P3",
							 .url = "www.github.com/p3",
							 .ownerID = "b2217c08-06eb-4a57-b977-7c6d72299301",
							 .hashes = {"137fed017b6159acc0af30d2c6b403a5", "23920776594c85fdc30cd96f928487f1",
										"959ee1ee12e6d6d87a4b6ee732aed9fc"}};
	ProjectOut project4_3 = {.projectID = 4,
							 .version = 5000000005000,
							 .license = "L4",
							 .name = "P4",
							 .url = "www.github.com/p4",
							 .ownerID = "e39e0872-6856-4fa0-8d9a-278728362f43",
							 .hashes = {"06f73d7ab46184c55bf4742b9428a4c0", "8811e6bedb87e90cef39de1179f3bd2e"}};
	std::vector<ProjectOut> p4_1 = {project4_1};
	std::vector<ProjectOut> p4_2 = {};
	std::vector<ProjectOut> p4_3 = {project4_2};
	std::vector<ProjectOut> p4_4 = {project4_3};

	std::string expected4_1 = "2?5000000001000?L2?P2?www.github.com/p2?68bd2db6-fe91-47d2-a134-cf82b104f547";
	std::string expected4_2 = "3?5000000002000?L3?P3?www.github.com/p3?b2217c08-06eb-4a57-b977-7c6d72299301";
	std::string expected4_3 = "4?5000000005000?L4?P4?www.github.com/p4?e39e0872-6856-4fa0-8d9a-278728362f43";
	std::vector<std::string> expected4 = {expected4_1, expected4_2, expected4_3};
	EXPECT_CALL(database, searchForProject(projectID4_1, version4_1)).WillOnce(testing::Return(p4_1));
	EXPECT_CALL(database, searchForProject(projectID4_2, version4_2)).WillOnce(testing::Return(p4_2));
	EXPECT_CALL(database, searchForProject(projectID4_3, version4_3)).WillOnce(testing::Return(p4_3));
	EXPECT_CALL(database, searchForProject(projectID4_4, version4_4)).WillOnce(testing::Return(p4_4));
	std::string output4 = handler.handleRequest("extp", input4, nullptr);
	std::vector<std::string> entries4 = Utility::splitStringOn(output4, '\n');

	// Assert that the output contains 3 entries.
	ASSERT_EQ(entries4.size(), 3);

	// Make sure that the entries are as expected.
	for (int i = 0; i < entries4.size(); i++)
	{
		std::vector<std::string>::iterator index4 = std::find(expected4.begin(), expected4.end(), entries4[i]);
		ASSERT_NE(index4, expected4.end());
	}
}

// Tests if an input for the extract projects request with too few arguments is correctly handled.
TEST(ExtractProjectsRequestTests, TooFewArguments)
{
	RequestHandler handler;

	std::string input5 = "0?0?\n1\n2?2";
	std::string expected5 =
		"The request failed. Each project should be provided a projectID and a version (in that order).";

	std::string output5 = handler.handleRequest("extp", input5, nullptr);
	ASSERT_EQ(output5, expected5);
}

// Tests if an input for the extract projects request with wrong argument types is correctly handled.
TEST(ExtractProjectsRequestTests, WrongArgumentTypes)
{
	RequestHandler handler;

	std::string input6 = "0?0?\n-?-\n2?2";
	std::string expected6 =
		"The request failed. For each project, both the projectID and the version should be a long long int.";

	std::string output6 = handler.handleRequest("extp", input6, nullptr);
	ASSERT_EQ(output6, expected6);
}

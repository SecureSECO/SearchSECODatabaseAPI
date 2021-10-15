cmake_minimum_required (VERSION 3.13)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.10.0
)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()


SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lboost_system")

include_directories(../external/include)
include_directories(Database-API)
include_directories(General)
include_directories(JobDistribution)

enable_testing()

add_executable(
	tests
	Database-API/AuthorRequest_test.cpp
	Database-API/CheckRequest_test.cpp
	Database-API/CheckUploadRequest_test.cpp
	Database-API/ExtractProjectsRequest_test.cpp
	Database-API/IntegrationTests.cpp
	Database-API/PrevProjectsRequest_test.cpp
	Database-API/UploadRequest_test.cpp
	Database-API/DatabaseMock.cpp
	General/ConnectionMock.cpp
	General/RequestHandlerMock.cpp
	General/HTTPStatus_test.cpp
	General/RequestHandler_test.cpp
	General/Utility_test.cpp
	JobDistribution/CrawlDataRequest_test.cpp
	JobDistribution/GetIPs_test.cpp
	JobDistribution/GetJobRequest_test.cpp
	JobDistribution/JobIntegrationTests.cpp
	JobDistribution/JobRequestHandler_test.cpp
	JobDistribution/Raft_test.cpp
	JobDistribution/UploadJobRequest_test.cpp
	JobDistribution/UpdateJobRequest_test.cpp
	JobDistribution/FinishJobRequest_test.cpp
	JobDistribution/JDDatabaseMock.cpp
	JobDistribution/RaftConsensusMock.cpp
)
target_link_libraries(
  tests
  gtest_main
  gmock
  "Database-API-library"
)
target_link_libraries(tests /usr/lib/x86_64-linux-gnu/libcassandra.so.2.15.3)
target_link_libraries(tests ${CURL_LIBRARIES})
target_link_libraries(tests prometheus-cpp::pull)

# Copying it twice, because the pipeline expects it to be somewhere else then running it localy.
configure_file("../dotenvTestfile.txt" "dotenvTestfile.txt" COPYONLY)
configure_file("../dotenvTestfile.txt" "../dotenvTestfile.txt" COPYONLY)

include(GoogleTest)
gtest_discover_tests(tests)

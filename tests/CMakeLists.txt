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

enable_testing()

add_executable(
	tests
	Raft_test.cpp
	GetIPs_test.cpp
	Utility_test.cpp
	HTTPStatus_test.cpp
	RequestHandler_test.cpp
	CheckRequest_test.cpp
	UploadRequest_test.cpp
	CheckUploadRequest_test.cpp
	ExtractProjectsRequest_test.cpp
	PrevProjectsRequest_test.cpp
	AuthorRequest_test.cpp
	IntegrationTests.cpp
	JobRequestHandler_test.cpp
	UploadJobRequest_test.cpp
	GetJobRequest_test.cpp
	CrawlDataRequest_test.cpp
	JobIntegrationTests.cpp
)
target_link_libraries(
  tests
  gtest_main
  gmock
  "Database-API-library"
)
target_link_libraries(tests /usr/lib/x86_64-linux-gnu/libcassandra.so.2.15.3)

# Copying it twice, because the pipeline expects it to be somewhere else then running it localy.
configure_file("../dotenvTestfile.txt" "dotenvTestfile.txt" COPYONLY)
configure_file("../dotenvTestfile.txt" "../dotenvTestfile.txt" COPYONLY)

include(GoogleTest)
gtest_discover_tests(tests)

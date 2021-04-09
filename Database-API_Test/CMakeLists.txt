
#project(tests)

cmake_minimum_required (VERSION 3.8)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.10.0
)

FetchContent_MakeAvailable(googletest)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

include_directories(../Resources/include)

enable_testing()

add_executable(
  tests
  RequestHandler_test.cpp
  CheckRequest_test.cpp
)
target_link_libraries(
  tests
  gtest_main
  gmock
  "Database-API-library"
)
target_link_libraries(tests /usr/lib/x86_64-linux-gnu/libcassandra.so.2.15.3)

include(GoogleTest)
gtest_discover_tests(tests)

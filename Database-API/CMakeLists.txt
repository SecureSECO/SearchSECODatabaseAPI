# CMakeList.txt : CMake project for Database-API, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (Database-API "Database-API.cpp" "Database-API.h" "DatabaseHandler.cpp" "DatabaseHandler.h" "Types.h" "ConnectionHandler.cpp" "RequestHandler.cpp" "Utility.h" "Utility.cpp")
add_library(Database-API-library "Database-API.cpp" "Database-API.h" "DatabaseHandler.cpp" "DatabaseHandler.h" "Types.h" "ConnectionHandler.cpp" "RequestHandler.cpp" "Utility.h" "Utility.cpp")


SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lboost_system")

include_directories(../Resources/include)
target_link_libraries(Database-API /usr/lib/x86_64-linux-gnu/libcassandra.so.2.15.3)

# TODO: Add tests and install targets if needed.

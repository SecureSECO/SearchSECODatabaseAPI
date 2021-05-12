# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("Database-API")

# Add source to this project's executable.
add_executable (Database-APIexe "General/Database-API.cpp" "General/Database-API.h" "Database-API/DatabaseHandler.cpp" "Database-API/DatabaseHandler.h" "Database-API/Types.h" "General/ConnectionHandler.cpp" "General/RequestHandler.cpp" "Database-API/Utility.h" "Database-API/Utility.cpp" "Database-API/DatabaseRequestHandler.cpp")
add_library(Database-API-library "General/Database-API.cpp" "General/Database-API.h" "Database-API/DatabaseHandler.cpp" "Database-API/DatabaseHandler.h" "Database-API/Types.h" "General/ConnectionHandler.cpp" "General/RequestHandler.cpp" "Database-API/Utility.h" "Database-API/Utility.cpp" "Database-API/DatabaseRequestHandler.cpp")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lboost_system")

include_directories(Resources/include)
target_link_libraries(Database-APIexe /usr/lib/x86_64-linux-gnu/libcassandra.so.2.15.3)

include_directories("Database-API")
include_directories("General")

IF(CMAKE_BUILD_TYPE MATCHES Debug)
include(Resources/CodeCoverage.cmake)
append_coverage_compiler_flags()

setup_target_for_coverage_lcov(
		NAME coverage
		EXECUTABLE tests
		EXCLUDE "/usr/include/*" "${PROJECT_SOURCE_DIR}/Database-API_Test/*" "${PROJECT_SOURCE_DIR}/build/_deps/*" "${PROJECT_SOURCE_DIR}/_deps/*")
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

# Include sub-projects.
add_subdirectory ("Database-API_Test")

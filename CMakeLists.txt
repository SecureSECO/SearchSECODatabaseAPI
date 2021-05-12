# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("Database-API")
# Add source to this project's executable.
add_executable (Database-APIexe
	"General/Database-API.cpp" "General/Database-API.h"
	"General/ConnectionHandler.cpp" "General/ConnectionHandler.h"
	"General/RequestHandler.cpp" "General/RequestHandler.h"
	"General/Utility.cpp" "General/Utility.h"
	"Database-API/DatabaseHandler.cpp" "Database-API/DatabaseHandler.h"
	"Database-API/Types.h" 
	"Database-API/DatabaseRequestHandler.cpp" "Database-API/DatabaseRequestHandler.h"
	"JobDistribution/JobRequestHandler.cpp" "JobDistribution/JobRequestHandler.h"
	"JobDistribution/DatabaseConnection.cpp" "JobDistribution/DatabaseConnection.h"
	"JobDistribution/Networking.cpp" "JobDistribution/Networking.h"
	"JobDistribution/RAFTConsensus.cpp" "JobDistribution/RAFTConsensus.h")
add_library(Database-API-library
	"General/Database-API.cpp" "General/Database-API.h"
	"General/ConnectionHandler.cpp" "General/ConnectionHandler.h"
	"General/RequestHandler.cpp" "General/RequestHandler.h"
	"General/Utility.cpp" "General/Utility.h"
	"Database-API/DatabaseHandler.cpp" "Database-API/DatabaseHandler.h"
	"Database-API/Types.h" 
	"Database-API/DatabaseRequestHandler.cpp" "Database-API/DatabaseRequestHandler.h"
	"JobDistribution/JobRequestHandler.cpp" "JobDistribution/JobRequestHandler.h"
	"JobDistribution/DatabaseConnection.cpp" "JobDistribution/DatabaseConnection.h"
	"JobDistribution/Networking.cpp" "JobDistribution/Networking.h"
	"JobDistribution/RAFTConsensus.cpp" "JobDistribution/RAFTConsensus.h")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lboost_system")

include_directories(Resources/include)
target_link_libraries(Database-APIexe /usr/lib/x86_64-linux-gnu/libcassandra.so.2.15.3)

include_directories("Database-API")
include_directories("General")
include_directories("JobDistribution")

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

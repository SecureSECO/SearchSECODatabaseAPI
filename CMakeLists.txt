cmake_minimum_required (VERSION 3.8)

project ("Database-API")

add_executable (Database-APIexe
	"libSearchSECODatabaseAPI/General/Database-API.cpp" "libSearchSECODatabaseAPI/General/Database-API.h"
	"libSearchSECODatabaseAPI/General/ConnectionHandler.cpp" "libSearchSECODatabaseAPI/General/ConnectionHandler.h"
	"libSearchSECODatabaseAPI/General/RequestHandler.cpp" "libSearchSECODatabaseAPI/General/RequestHandler.h"
	"libSearchSECODatabaseAPI/General/Utility.cpp" "libSearchSECODatabaseAPI/General/Utility.h"
	"libSearchSECODatabaseAPI/General/HTTPStatus.cpp" "libSearchSECODatabaseAPI/General/HTTPStatus.h"
	"libSearchSECODatabaseAPI/General/Definitions.h"
	"libSearchSECODatabaseAPI/Database-API/DatabaseHandler.cpp" "libSearchSECODatabaseAPI/Database-API/DatabaseHandler.h"
	"libSearchSECODatabaseAPI/Database-API/Types.h"
	"libSearchSECODatabaseAPI/Database-API/DatabaseRequestHandler.cpp" "libSearchSECODatabaseAPI/Database-API/DatabaseRequestHandlerAuthor.cpp" 
	"libSearchSECODatabaseAPI/Database-API/DatabaseRequestHandlerMethod.cpp" "libSearchSECODatabaseAPI/Database-API/DatabaseRequestHandlerProject.cpp" 
	"libSearchSECODatabaseAPI/Database-API/DatabaseRequestHandler.h"
	"libSearchSECODatabaseAPI/General/md5/md5.cpp" "libSearchSECODatabaseAPI/General/md5/md5.h"
	"libSearchSECODatabaseAPI/JobDistribution/JobRequestHandler.cpp" "libSearchSECODatabaseAPI/JobDistribution/JobRequestHandler.h"
	"libSearchSECODatabaseAPI/JobDistribution/DatabaseConnection.cpp" "libSearchSECODatabaseAPI/JobDistribution/DatabaseConnection.h"
	"libSearchSECODatabaseAPI/JobDistribution/Networking.cpp" "libSearchSECODatabaseAPI/JobDistribution/Networking.h"
	"libSearchSECODatabaseAPI/JobDistribution/RAFTConsensus.cpp" "libSearchSECODatabaseAPI/JobDistribution/RAFTConsensus.h")
add_library(Database-API-library
	"libSearchSECODatabaseAPI/General/Database-API.cpp" "libSearchSECODatabaseAPI/General/Database-API.h"
	"libSearchSECODatabaseAPI/General/ConnectionHandler.cpp" "libSearchSECODatabaseAPI/General/ConnectionHandler.h"
	"libSearchSECODatabaseAPI/General/RequestHandler.cpp" "libSearchSECODatabaseAPI/General/RequestHandler.h"
	"libSearchSECODatabaseAPI/General/Utility.cpp" "libSearchSECODatabaseAPI/General/Utility.h"
	"libSearchSECODatabaseAPI/General/HTTPStatus.cpp" "libSearchSECODatabaseAPI/General/HTTPStatus.h"
	"libSearchSECODatabaseAPI/Database-API/DatabaseHandler.cpp" "libSearchSECODatabaseAPI/Database-API/DatabaseHandler.h"
	"libSearchSECODatabaseAPI/Database-API/Types.h"
	"libSearchSECODatabaseAPI/Database-API/DatabaseRequestHandler.cpp" "libSearchSECODatabaseAPI/Database-API/DatabaseRequestHandlerAuthor.cpp" 
	"libSearchSECODatabaseAPI/Database-API/DatabaseRequestHandlerMethod.cpp" "libSearchSECODatabaseAPI/Database-API/DatabaseRequestHandlerProject.cpp" 
	"libSearchSECODatabaseAPI/Database-API/DatabaseRequestHandler.h"
	"libSearchSECODatabaseAPI/General/md5/md5.cpp" "libSearchSECODatabaseAPI/General/md5/md5.h"
	"libSearchSECODatabaseAPI/JobDistribution/JobRequestHandler.cpp" "libSearchSECODatabaseAPI/JobDistribution/JobRequestHandler.h"
	"libSearchSECODatabaseAPI/JobDistribution/DatabaseConnection.cpp" "libSearchSECODatabaseAPI/JobDistribution/DatabaseConnection.h"
	"libSearchSECODatabaseAPI/JobDistribution/Networking.cpp" "libSearchSECODatabaseAPI/JobDistribution/Networking.h"
	"libSearchSECODatabaseAPI/JobDistribution/RAFTConsensus.cpp" "libSearchSECODatabaseAPI/JobDistribution/RAFTConsensus.h")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lboost_system")

include_directories(external/include)
target_link_libraries(Database-APIexe /usr/lib/x86_64-linux-gnu/libcassandra.so.2.15.3)

include_directories("libSearchSECODatabaseAPI/Database-API")
include_directories("libSearchSECODatabaseAPI/General")
include_directories("libSearchSECODatabaseAPI/JobDistribution")

IF(CMAKE_BUILD_TYPE MATCHES Debug)
include(external/CodeCoverage.cmake)
append_coverage_compiler_flags()

setup_target_for_coverage_lcov(
		NAME coverage
		EXECUTABLE tests
		EXCLUDE "/usr/include/*" "${PROJECT_SOURCE_DIR}../tests/*" "${PROJECT_SOURCE_DIR}../build/_deps/*" "${PROJECT_SOURCE_DIR}../_deps/*")
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

# Copy .env file
configure_file(".env" ".env" COPYONLY)

# Include sub-projects.
add_subdirectory ("tests")

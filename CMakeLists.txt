cmake_minimum_required (VERSION 3.8)

project ("Database-API")

add_executable (Database-APIexe
	"SearchSECODatabaseAPI/General/Database-API.cpp" "SearchSECODatabaseAPI/General/Database-API.h"
	"SearchSECODatabaseAPI/General/Statistics.cpp" "SearchSECODatabaseAPI/General/Statistics.h"
	"SearchSECODatabaseAPI/General/ConnectionHandler.cpp" "SearchSECODatabaseAPI/General/ConnectionHandler.h"
	"SearchSECODatabaseAPI/General/RequestHandler.cpp" "SearchSECODatabaseAPI/General/RequestHandler.h"
	"SearchSECODatabaseAPI/General/Utility.cpp" "SearchSECODatabaseAPI/General/Utility.h"
	"SearchSECODatabaseAPI/General/DatabaseUtility.cpp" "SearchSECODatabaseAPI/General/DatabaseUtility.h"
	"SearchSECODatabaseAPI/General/HTTPStatus.cpp" "SearchSECODatabaseAPI/General/HTTPStatus.h"
	"SearchSECODatabaseAPI/General/Definitions.h"
	"SearchSECODatabaseAPI/Database-API/DatabaseHandlerGet.cpp" "SearchSECODatabaseAPI/Database-API/DatabaseHandlerSet.cpp"
	"SearchSECODatabaseAPI/Database-API/DatabaseHandler.cpp" "SearchSECODatabaseAPI/Database-API/DatabaseHandler.h"
	"SearchSECODatabaseAPI/Database-API/Types.h"
	"SearchSECODatabaseAPI/Database-API/DatabaseRequestHandler.cpp" "SearchSECODatabaseAPI/Database-API/DatabaseRequestHandlerAuthor.cpp" 
	"SearchSECODatabaseAPI/Database-API/DatabaseRequestHandlerMethod.cpp" "SearchSECODatabaseAPI/Database-API/DatabaseRequestHandlerProject.cpp" 
	"SearchSECODatabaseAPI/Database-API/DatabaseRequestHandler.h"
	"SearchSECODatabaseAPI/General/md5/md5.cpp" "SearchSECODatabaseAPI/General/md5/md5.h"
	"SearchSECODatabaseAPI/JobDistribution/JobTypes.h"
	"SearchSECODatabaseAPI/JobDistribution/JobRequestHandler.cpp" "SearchSECODatabaseAPI/JobDistribution/JobRequestHandler.h"
	"SearchSECODatabaseAPI/JobDistribution/DatabaseConnection.cpp" "SearchSECODatabaseAPI/JobDistribution/DatabaseConnection.h"
	"SearchSECODatabaseAPI/JobDistribution/Networking.cpp" "SearchSECODatabaseAPI/JobDistribution/Networking.h"
	"SearchSECODatabaseAPI/JobDistribution/RAFTConsensus.cpp" "SearchSECODatabaseAPI/JobDistribution/RAFTConsensus.h")
add_library(Database-API-library
	"SearchSECODatabaseAPI/General/Database-API.cpp" "SearchSECODatabaseAPI/General/Database-API.h"
	"SearchSECODatabaseAPI/General/Statistics.cpp" "SearchSECODatabaseAPI/General/Statistics.h"
	"SearchSECODatabaseAPI/General/ConnectionHandler.cpp" "SearchSECODatabaseAPI/General/ConnectionHandler.h"
	"SearchSECODatabaseAPI/General/RequestHandler.cpp" "SearchSECODatabaseAPI/General/RequestHandler.h"
	"SearchSECODatabaseAPI/General/Utility.cpp" "SearchSECODatabaseAPI/General/Utility.h"
	"SearchSECODatabaseAPI/General/DatabaseUtility.cpp" "SearchSECODatabaseAPI/General/DatabaseUtility.h"
	"SearchSECODatabaseAPI/General/HTTPStatus.cpp" "SearchSECODatabaseAPI/General/HTTPStatus.h"
	"SearchSECODatabaseAPI/Database-API/DatabaseHandlerGet.cpp" "SearchSECODatabaseAPI/Database-API/DatabaseHandlerSet.cpp"
	"SearchSECODatabaseAPI/Database-API/DatabaseHandler.cpp" "SearchSECODatabaseAPI/Database-API/DatabaseHandler.h"
	"SearchSECODatabaseAPI/Database-API/Types.h"
	"SearchSECODatabaseAPI/Database-API/DatabaseRequestHandler.cpp" "SearchSECODatabaseAPI/Database-API/DatabaseRequestHandlerAuthor.cpp" 
	"SearchSECODatabaseAPI/Database-API/DatabaseRequestHandlerMethod.cpp" "SearchSECODatabaseAPI/Database-API/DatabaseRequestHandlerProject.cpp" 
	"SearchSECODatabaseAPI/Database-API/DatabaseRequestHandler.h"
	"SearchSECODatabaseAPI/General/md5/md5.cpp" "SearchSECODatabaseAPI/General/md5/md5.h"
	"SearchSECODatabaseAPI/JobDistribution/JobTypes.h"
	"SearchSECODatabaseAPI/JobDistribution/JobRequestHandler.cpp" "SearchSECODatabaseAPI/JobDistribution/JobRequestHandler.h"
	"SearchSECODatabaseAPI/JobDistribution/DatabaseConnection.cpp" "SearchSECODatabaseAPI/JobDistribution/DatabaseConnection.h"
	"SearchSECODatabaseAPI/JobDistribution/Networking.cpp" "SearchSECODatabaseAPI/JobDistribution/Networking.h"
	"SearchSECODatabaseAPI/JobDistribution/RAFTConsensus.cpp" "SearchSECODatabaseAPI/JobDistribution/RAFTConsensus.h")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lboost_system")

include_directories(external/include)
target_link_libraries(Database-APIexe /usr/lib/x86_64-linux-gnu/libcassandra.so.2.15.3)

include_directories("SearchSECODatabaseAPI/Database-API")
include_directories("SearchSECODatabaseAPI/General")
include_directories("SearchSECODatabaseAPI/JobDistribution")

set(CURL_LIBRARY "-lcurl")
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})
target_link_libraries(Database-APIexe ${CURL_LIBRARIES})

find_package(prometheus-cpp CONFIG REQUIRED)

target_link_libraries(Database-APIexe prometheus-cpp::pull)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
include(external/CodeCoverage.cmake)
append_coverage_compiler_flags()

setup_target_for_coverage_lcov(
		NAME coverage
		EXECUTABLE tests
		EXCLUDE "/usr/include/*" "${PROJECT_SOURCE_DIR}../tests/*" "${PROJECT_SOURCE_DIR}../build/_deps/*" "${PROJECT_SOURCE_DIR}../_deps/*" "${PROJECT_SOURCE_DIR}../SearchSECODatabaseAPI/General/md5/*")
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

# Copy .env file
configure_file(".env" ".env" COPYONLY)

# Include sub-projects.
# add_subdirectory ("tests")

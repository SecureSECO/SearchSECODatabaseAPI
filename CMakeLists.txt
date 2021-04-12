# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("Database-API")

include_directories("Database-API")
IF(CMAKE_BUILD_TYPE MATCHES Debug)
include(CodeCoverage.cmake)
append_coverage_compiler_flags()

setup_target_for_coverage_lcov(
          NAME coverage
          EXECUTABLE tests
		  EXCLUDE "/usr/include/*" "${PROJECT_SOURCE_DIR}/Database-API_Test/*" "${PROJECT_SOURCE_DIR}/build/_deps/*" "${PROJECT_SOURCE_DIR}/_deps/*")
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)
# Include sub-projects.
add_subdirectory ("Database-API")
add_subdirectory ("Database-API_Test")

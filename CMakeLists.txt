# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("Database-API")

include_directories("Database-API")

# Include sub-projects.
add_subdirectory ("Database-API")
add_subdirectory ("Database-API_Test")

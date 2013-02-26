cmake_minimum_required (VERSION 2.8)
project(ebftpd)
include ("../../cmake/Defaults.cmake")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -DEXTERNAL_TOOL")

# Configure OpenSSL library
find_package (OpenSSL REQUIRED)
link_directories (${OpenSSL_LIBRARY_DIRS})
include_directories (${OpenSSL_INCLUDE_DIRS})

include_directories (src ${SERVER_SRC} ../../util)
add_executable (ranks ranks.cpp)

add_dependencies(ranks version)
target_link_libraries(ranks eb util ${ALL_LIBRARIES})

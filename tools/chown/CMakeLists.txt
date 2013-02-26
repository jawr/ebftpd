cmake_minimum_required (VERSION 2.8)
project(ebftpd)
include ("../../cmake/Defaults.cmake")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
include_directories (${SERVER_SRC} ../../util)
add_executable (chown chown.cpp)
add_dependencies(chown version)
target_link_libraries(chown eb util ${ALL_LIBRARIES})


cmake_minimum_required(VERSION 2.8)
project(ebftpd)
include("../../cmake/Defaults.cmake")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic")
include_directories(${SERVER_SRC} ../../util ..)

add_library(test SHARED EXCLUDE_FROM_ALL plugin.cpp)
set_target_properties(test PROPERTIES PREFIX "")
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -DSOURCE=${CMAKE_BINARY_DIR}/lib/liblua.so -DDEST=${CMAKE_INSTALL_PREFIX}/plugins/liblua.so -P ../cmake/OptionalInstall.cmake)")


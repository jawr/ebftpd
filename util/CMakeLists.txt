cmake_minimum_required (VERSION 2.8)
project("ebftpd")
include("../cmake/Defaults.cmake")
include_directories(.)

file(GLOB_RECURSE SRCS util/*.cpp)
add_library(util SHARED ${SRCS})

install(TARGETS util
        LIBRARY DESTINATION lib)

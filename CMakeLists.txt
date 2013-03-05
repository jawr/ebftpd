cmake_minimum_required (VERSION 2.8)
project("ebftpd")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set (CMAKE_INSTALL_PREFIX "/ebftpd" CACHE STRING "Install path" FORCE)
endif()
include("cmake/Defaults.cmake")
add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(util)
add_subdirectory(data)
add_subdirectory(plugins)

install(FILES ebftpd.conf.example DESTINATION etc)


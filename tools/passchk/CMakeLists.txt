cmake_minimum_required (VERSION 2.8)
project(ebftpd)
include ("../../cmake/Defaults.cmake")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -DEXTERNAL_TOOL")

# Configure OpenSSL library
find_package (OpenSSL REQUIRED)
link_directories (${OpenSSL_LIBRARY_DIRS})
include_directories (${OpenSSL_INCLUDE_DIRS})

include_directories (src ${SERVER_SRC})
add_executable (passchk src/passchk.cpp 
                ${SERVER_SRC}/util/passwd.cpp
                ${SERVER_SRC}/cfg/config.cpp
                ${SERVER_SRC}/cfg/setting.cpp
                ${SERVER_SRC}/cfg/section.cpp
                ${SERVER_SRC}/util/string.cpp
                ${SERVER_SRC}/util/error.cpp
                ${SERVER_SRC}/acl/acl.cpp
                ${SERVER_SRC}/acl/permission.cpp
                ${SERVER_SRC}/logs/logs.cpp
                ${SERVER_SRC}/logs/logger.cpp
                ${SERVER_SRC}/logs/streamsink.cpp
                ${SERVER_SRC}/logs/filesink.cpp
                ${SERVER_SRC}/cfg/get.cpp
                ${SERVER_SRC}/util/path/path.cpp)

add_dependencies(passchk version)
target_link_libraries(
  passchk
  ${MongoDB_LIBRARIES}
  ${Boost_LIBRARIES}
  ${Pthread_LIBRARIES}
  ${OPENSSL_LIBRARIES}
)

cmake_minimum_required (VERSION 2.8)
project(ebftpd)
include ("../../cmake/Defaults.cmake")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -DEXTERNAL_TOOL")

# Configure OpenSSL library
find_package (OpenSSL REQUIRED)
link_directories (${OpenSSL_LIBRARY_DIRS})
include_directories (${OpenSSL_INCLUDE_DIRS})

include_directories (src ${SERVER_SRC})
add_executable (ranks src/ranks.cpp 
                ${SERVER_SRC}/acl/flags.cpp
                ${SERVER_SRC}/util/passwd.cpp
                ${SERVER_SRC}/acl/user.cpp
                ${SERVER_SRC}/acl/group.cpp
                ${SERVER_SRC}/db/user/user.cpp
                ${SERVER_SRC}/db/group/group.cpp
                ${SERVER_SRC}/db/connection.cpp
                ${SERVER_SRC}/db/serialization.cpp
                ${SERVER_SRC}/db/stats/stats.cpp
                ${SERVER_SRC}/db/stats/serialization.cpp
                ${SERVER_SRC}/cfg/config.cpp
                ${SERVER_SRC}/cfg/get.cpp
                ${SERVER_SRC}/cfg/setting.cpp
                ${SERVER_SRC}/cfg/section.cpp
                ${SERVER_SRC}/util/string.cpp
                ${SERVER_SRC}/util/error.cpp
                ${SERVER_SRC}/util/path/path.cpp
                ${SERVER_SRC}/util/path/diriterator.cpp
                ${SERVER_SRC}/acl/acl.cpp
                ${SERVER_SRC}/acl/permission.cpp
                ${SERVER_SRC}/util/path/path.cpp
                ${SERVER_SRC}/stats/types.cpp
                ${SERVER_SRC}/stats/compile.cpp
                ${SERVER_SRC}/text/factory.cpp
                ${SERVER_SRC}/text/tag.cpp
                ${SERVER_SRC}/text/templatesection.cpp
                ${SERVER_SRC}/text/parser.cpp
                ${SERVER_SRC}/logs/logs.cpp
                ${SERVER_SRC}/logs/logger.cpp
                ${SERVER_SRC}/logs/streamsink.cpp
                ${SERVER_SRC}/logs/filesink.cpp
                ${SERVER_SRC}/util/titlecase.cpp
                ${SERVER_SRC}/db/user/util.cpp
                ${SERVER_SRC}/db/group/util.cpp)

add_dependencies(ranks version)
target_link_libraries(
  ranks
  ${MongoDB_LIBRARIES}
  ${Boost_LIBRARIES}
  ${Pthread_LIBRARIES}
  ${OPENSSL_LIBRARIES}
)

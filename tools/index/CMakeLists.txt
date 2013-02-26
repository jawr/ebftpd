cmake_minimum_required (VERSION 2.8)
project(ebftpd)
include ("../../cmake/Defaults.cmake")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DEXTERNAL_TOOL")
include_directories (src ${SERVER_SRC})
add_executable (index src/index.cpp
                ${SERVER_SRC}/cfg/config.cpp
                ${SERVER_SRC}/cfg/setting.cpp
                ${SERVER_SRC}/cfg/section.cpp
                ${SERVER_SRC}/acl/acl.cpp
                ${SERVER_SRC}/util/string.cpp
                ${SERVER_SRC}/acl/permission.cpp
                ${SERVER_SRC}/util/path/path.cpp
                ${SERVER_SRC}/util/path/status.cpp
                ${SERVER_SRC}/util/path/diriterator.cpp
                ${SERVER_SRC}/util/error.cpp
                ${SERVER_SRC}/util/path/globiterator.cpp
                ${SERVER_SRC}/util/path/recursivediriterator.cpp)
add_dependencies(index version)
target_link_libraries(
  index
  ${MongoDB_LIBRARIES}
  ${Boost_LIBRARIES}
  ${Pthread_LIBRARIES}
)

cmake_minimum_required (VERSION 2.8)
project(ebftpd)
include ("../../CMakeModules/Defaults.cmake")
include_directories (src ${SERVER_SRC})
add_executable (chown src/chown.cpp
								${SERVER_SRC}/cfg/config.cpp
								${SERVER_SRC}/cfg/setting.cpp
								${SERVER_SRC}/cfg/section.cpp
								${SERVER_SRC}/logs/logs.cpp
								${SERVER_SRC}/util/logger.cpp
								${SERVER_SRC}/util/path/path.cpp
								${SERVER_SRC}/acl/acl.cpp
								${SERVER_SRC}/acl/permission.cpp
								${SERVER_SRC}/util/string.cpp
								${SERVER_SRC}/acl/flags.cpp
								${SERVER_SRC}/fs/owner.cpp
								${SERVER_SRC}/util/error.cpp
								${SERVER_SRC}/cfg/get.cpp
								${SERVER_SRC}/util/path/status.cpp
								${SERVER_SRC}/util/path/diriterator.cpp)
add_dependencies(chown version)
target_link_libraries(
  chown
  ${MongoDB_LIBRARIES}
  ${Boost_LIBRARIES}
  ${Pthread_LIBRARIES}
)


cmake_minimum_required(VERSION 2.8)
project(ebftpd)
include("../../cmake/Defaults.cmake")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic")
include_directories(${SERVER_SRC} ../../util ..)

find_package(Lua51 REQUIRED)
if(LUA51_FOUND)
  include_directories(${LUA_INCLUDE_DIR})
  add_library(lua SHARED EXCLUDE_FROM_ALL plugin.cpp)
  add_dependencies(lua eb util)
	set_target_properties(lua PROPERTIES PREFIX "")
  target_link_libraries(lua eb util ${LUA_LIBRARIES} luabind)
	install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -DSOURCE=${CMAKE_BINARY_DIR}/lib/liblua.so -DDEST=${CMAKE_INSTALL_PREFIX}/plugins/liblua.so -P ../cmake/OptionalInstall.cmake)")

else()
  message(WARNING "Skipping building lua plugin")
endif()


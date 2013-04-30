cmake_minimum_required (VERSION 2.8)
project("ebftpd")
include("../cmake/Defaults.cmake")
include_directories(. ../util)

# Configure OpenSSL library
find_package (OpenSSL REQUIRED)
link_directories (${OpenSSL_LIBRARY_DIRS})
include_directories (${OpenSSL_INCLUDE_DIRS})

add_executable(ebftpd main.cpp)
add_dependencies(ebftpd version util eb)
target_link_libraries(
  ebftpd util eb
  ${ALL_LIBRARIES}
)

file(GLOB_RECURSE SUBSRCS acl/*.cpp)
list(APPEND SRCS ${SUBSRCS})

file(GLOB_RECURSE SUBSRCS db/*.cpp)
list(APPEND SRCS ${SUBSRCS})

file(GLOB_RECURSE SUBSRCS cfg/*.cpp)
list(APPEND SRCS ${SUBSRCS})

file(GLOB_RECURSE SUBSRCS cmd/*.cpp)
list(APPEND SRCS ${SUBSRCS})

file(GLOB_RECURSE SUBSRCS exec/*.cpp)
list(APPEND SRCS ${SUBSRCS})

file(GLOB_RECURSE SUBSRCS fs/*.cpp)
list(APPEND SRCS ${SUBSRCS})

file(GLOB_RECURSE SUBSRCS ftp/*.cpp)
list(APPEND SRCS ${SUBSRCS})

file(GLOB_RECURSE SUBSRCS logs/*.cpp)
list(APPEND SRCS ${SUBSRCS})

file(GLOB_RECURSE SUBSRCS signals/*.cpp)
list(APPEND SRCS ${SUBSRCS})

file(GLOB_RECURSE SUBSRCS stats/*.cpp)
list(APPEND SRCS ${SUBSRCS})

file(GLOB_RECURSE SUBSRCS text/*.cpp)
list(APPEND SRCS ${SUBSRCS})

add_library(eb SHARED version.cpp ${SRCS})
add_dependencies(eb version)

install(TARGETS ebftpd eb RUNTIME DESTINATION bin LIBRARY DESTINATION lib)

install(
  CODE "message(STATUS \"Installing: ${CMAKE_INSTALL_PREFIX}/ebftpd -> ${CMAKE_INSTALL_PREFIX}/bin/ebftpd\")"
  CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CMAKE_INSTALL_PREFIX}/bin/ebftpd\" \"${CMAKE_INSTALL_PREFIX}/ebftpd\")"
  CODE "execute_process(COMMAND ${CMAKE_COMMAND} -DDEST=${CMAKE_INSTALL_PREFIX}/etc/ebftpd.pem -P ${MODULES_DIR}/GenerateCert.cmake)"
)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/dummy ${CMAKE_BINARY_DIR}/dummy/site)
install(DIRECTORY ${CMAKE_BINARY_DIR}/dummy/site DESTINATION .)

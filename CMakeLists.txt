cmake_minimum_required (VERSION 2.8)
project("ebftpd")
include("cmake/Defaults.cmake")
option(UNITY "unity builds" OFF)
project(ebftpd)
include_directories(src)

# Configure OpenSSL library
find_package (OpenSSL REQUIRED)
link_directories (${OpenSSL_LIBRARY_DIRS})
include_directories (${OpenSSL_INCLUDE_DIRS})

if(${UNITY} GREATER 0)
  set(COUNT 0)
  while(${COUNT} LESS ${UNITY})
    MATH(EXPR COUNT "${COUNT} + 1")
    set(SRC ${CMAKE_CURRENT_BINARY_DIR}/unity/unity-${COUNT}.cpp)
    if(NOT EXISTS ${SRC})
      file(WRITE ${SRC} "")
    endif()
    list(APPEND SRCS ${SRC})
  endwhile()
  add_custom_target(prepare_unity 
                    COMMAND ${UNITY_SCRIPT} ${CMAKE_CURRENT_SOURCE_DIR}/src 
                    ${CMAKE_CURRENT_BINARY_DIR}/unity unity ${UNITY})
  add_executable(ebftpd ${SRCS})
  add_dependencies(ebftpd version prepare_unity)

else()
  file(GLOB_RECURSE SRCS src/*.cpp) 
  add_executable(ebftpd ${SRCS})
  add_dependencies(ebftpd version)

endif()

target_link_libraries(
  ebftpd
  ${MongoDB_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  ${Boost_LIBRARIES}
  ${Execinfo_LIBRARIES}
  ${Pthread_LIBRARIES}
)

add_subdirectory(tools)

cmake_minimum_required (VERSION 2.8)
project("ebftpd")
include("cmake/Defaults.cmake")
include("cmake/Cotire.cmake")
project(ebftpd)
include_directories(src)

# Configure OpenSSL library
find_package (OpenSSL REQUIRED)
link_directories (${OpenSSL_LIBRARY_DIRS})
include_directories (${OpenSSL_INCLUDE_DIRS})


include(CheckSymbolExists)
check_symbol_exists(PTRACE_TRACEME "sys/ptrace.h" HAVE_PTRACEME)
check_symbol_exists(PTRACE_TRACE_ME "sys/ptrace.h" HAVE_PTRACE_ME)

link_libraries(
  ${MongoDB_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  ${Boost_LIBRARIES}
  ${Execinfo_LIBRARIES}
  ${Pthread_LIBRARIES}
)
file(GLOB_RECURSE SRCS src/*.cpp) 
add_executable(ebftpd ${SRCS})
add_dependencies(ebftpd version)
cotire(ebftpd)

add_subdirectory(tools)

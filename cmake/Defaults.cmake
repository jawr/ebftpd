# Variable definitions
set (CMAKE_EXE_LINKER_FLAGS "-Wl,-z,origin")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (MODULES_DIR ${CMAKE_CURRENT_LIST_DIR})
set (CMAKE_MODULE_PATH ${MODULES_DIR} ${CMAKE_MODULE_PATH})
set (SCRIPTS_DIR ${CMAKE_CURRENT_LIST_DIR}/../scripts)
set (UNITY_SCRIPT ${SCRIPTS_DIR}/unity.sh)
set (CMAKE_CXX_FLAGS "-Wnon-virtual-dtor -Wall -Wextra -g -ggdb -std=c++0x -pedantic -Winit-self -Wno-deprecated")
set (SERVER_SRC ${CMAKE_CURRENT_LIST_DIR}/../src)

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "\$ORIGIN/lib:\$ORIGIN/:\$ORIGIN/../lib")

# Configure OpenSSL library
find_package (OpenSSL REQUIRED)
link_directories (${OpenSSL_LIBRARY_DIRS})
include_directories (${OpenSSL_INCLUDE_DIRS})

# Configure BOOST libraries
if (NOT Boost_LIBRARY_DIRS)
  set (Boost_USE_STATIC_LIBS OFF)
  set (Boost_USE_MULTITHREADED ON)
  set (Boost_USE_STATIC_RUNTIME OFF)
  find_package (Boost 1.49.0 REQUIRED  
                thread regex iostreams system filesystem 
                date_time program_options signals)
  set (Boost_LIBRARY_DIRS ${Boost_LIBRARY_DIRS} CACHE FILEPATH "Boost include directory")
  set (Boost_LIBRARIES ${Boost_LIBRARIES} CACHE STRING "Boost library list")
endif()

link_directories (${Boost_LIBRARY_DIRS})
include_directories (${Boost_INCLUDE_DIRS})

# Configure MongoDB client library
find_package (MongoDB REQUIRED)
include_directories (${MongoDB_INCLUDE_DIRS})

# Check for pthreads
find_package (Pthread REQUIRED)
include_directories (${Pthread_INCLUDE_DIRS})

# Some OSes seem to use external libexecinfo
find_package (Execinfo REQUIRED)
include_directories(${Execinfo_INCLUDE_DIRS})

if(NOT TARGET version)
	add_custom_target(version 
		COMMAND ${CMAKE_COMMAND} -D DEST=${CMAKE_CURRENT_LIST_DIR}/../src/version.hpp 
														 -P ${MODULES_DIR}/Version.cmake)
endif()

list(APPEND ALL_LIBRARIES 
  ${MongoDB_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  ${Boost_LIBRARIES}
  ${Execinfo_LIBRARIES}
  ${Pthread_LIBRARIES}
  rt
)

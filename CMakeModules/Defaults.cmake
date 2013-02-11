# Variable definitions
set (MODULES_DIR ${CMAKE_CURRENT_LIST_DIR})
set (CMAKE_MODULE_PATH ${MODULES_DIR} ${CMAKE_MODULE_PATH})
set (SCRIPTS_DIR ${CMAKE_CURRENT_LIST_DIR}/../scripts)
set (UNITY_SCRIPT ${SCRIPTS_DIR}/unity.sh)
set (CMAKE_CXX_FLAGS "-Wnon-virtual-dtor -Wall -Wextra -g -ggdb -std=gnu++0x -pedantic -Winit-self")
set (SERVER_SRC ${CMAKE_CURRENT_LIST_DIR}/../src)

# Configure BOOST libraries
set (Boost_USE_STATIC_LIBS OFF)
set (Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_RUNTIME OFF)
find_package (Boost 1.49.0 REQUIRED  
              thread regex iostreams system filesystem 
              date_time program_options signals)
link_directories (${Boost_LIBRARY_DIRS})
include_directories (${Boost_INCLUDE_DIRS})

# Configure MongoDB client library
find_package (MongoDB REQUIRED)
include_directories (${MongoDB_INCLUDE_DIRS})

if(NOT TARGET version)
	add_custom_target(version 
		COMMAND ${CMAKE_COMMAND} -D DEST=${CMAKE_CURRENT_LIST_DIR}/../src/version.hpp 
														 -P ${MODULES_DIR}/version.cmake)
endif()

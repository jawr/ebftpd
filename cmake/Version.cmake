cmake_minimum_required (VERSION 2.8)
find_package(Git QUIET REQUIRED)
execute_process(COMMAND ${GIT_EXECUTABLE} log --decorate --all
                OUTPUT_VARIABLE GIT_OUTPUT)
string(REPLACE "\n" ";" GIT_LINES ${GIT_OUTPUT})

set(PATCH 0)
foreach(LINE ${GIT_LINES})
	if(${LINE} MATCHES "^commit ")
		if (${LINE} MATCHES "tag: ([0-9][0-9]*\\.[0-9][0-9]*)")
			set(VERS "${CMAKE_MATCH_1}")
			break()
		endif()
		MATH(EXPR PATCH "${PATCH} + 1")
	endif()
endforeach()

if(NOT VERS)
	set (VERS "0.0")
endif()

if(${PATCH} GREATER 0)
	set (VERS "${VERS}-${PATCH}")
endif()

if(EXISTS ${DEST})
	file(READ ${DEST} DEST_CONTENTS)
	if ("${DEST_CONTENTS}" MATCHES "\"${VERS}\"")
		return()
	endif()
endif()

file(WRITE ${DEST} "#define EBFTPD_VERSION \"${VERS}\"\n")

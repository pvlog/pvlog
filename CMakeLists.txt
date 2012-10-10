cmake_minimum_required(VERSION 2.8)
project(PvlogProject)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(PVLOG_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

if (CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_CXX_FLAGS "-std=c++0x -Wall -pedantic")
endif (CMAKE_COMPILER_IS_GNUCC)

find_package(sqlite3)

###############
# Unit testing
###############
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure)
add_subdirectory(gtest)
include_directories(gtest)

enable_testing()
macro(add_unittest TESTNAME SRC LIBS)
	message(test ${CMAKE_CURRENT_SOURCE_DIR})
	add_executable(${TESTNAME} EXCLUDE_FROM_ALL ${SRC})
	set_target_properties(${TESTNAME} PROPERTIES COMPILE_DEFINITIONS 
		"TEST_DIR_PATH=\"${CMAKE_CURRENT_BINARY_DIR}\"")
	target_link_libraries(${TESTNAME} ${LIBS} gtest)
	add_test(${TESTNAME} ${TESTNAME})
	add_dependencies(check ${TESTNAME})
endmacro(add_unittest)


add_subdirectory(src)



cmake_minimum_required(VERSION 2.8)
project(PvlogProject)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(PVLOG_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

if (CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -pedantic -Wno-unknown-pragmas")
	#odb is compiled with gcc 4.9 on ubuntu 16.04 so we need to use old api
	add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
endif (CMAKE_COMPILER_IS_GNUCC)

find_package(Boost REQUIRED COMPONENTS date_time)
find_package(sqlite3 REQUIRED)
find_package(ODB REQUIRED COMPONENTS sqlite boost)

###############
# Unit testing
###############
#add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure)
#add_subdirectory(gtest)
#include_directories(gtest)
#
#enable_testing()
macro(add_unittest TESTNAME SRC LIBS)
#	message(test ${CMAKE_CURRENT_SOURCE_DIR})
#	add_executable(${TESTNAME} EXCLUDE_FROM_ALL ${SRC})
#	set_target_properties(${TESTNAME} PROPERTIES COMPILE_DEFINITIONS 
#		"TEST_DIR_PATH=\"${CMAKE_CURRENT_BINARY_DIR}\"")
#	target_link_libraries(${TESTNAME} ${LIBS} gtest)
#	add_test(${TESTNAME} ${TESTNAME})
#	add_dependencies(check ${TESTNAME})
endmacro(add_unittest)


add_subdirectory(src)



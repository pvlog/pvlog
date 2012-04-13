cmake_minimum_required(VERSION 2.8)
project(PvlogProject)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
MESSAGE("SOURCE_DIR:  "${CMAKE_MODULE_PATH})

set(PVLOG_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

if (CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_CXX_FLAGS "-std=c++0x -Wall -pedantic")
endif (CMAKE_COMPILER_IS_GNUCC)

find_package(sqlite3)

find_Package(GTest)

add_subdirectory(pvlib)
include_directories(pvlib/src)

add_subdirectory(src)
add_subdirectory(databasesetup EXCLUDE_FROM_ALL)

if (GTEST_FOUND)
	#	enable_testing()
	add_subdirectory(tests EXCLUDE_FROM_ALL)
endif (GTEST_FOUND)


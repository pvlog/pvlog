cmake_minimum_required(VERSION 2.8)

project(Pvlog)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
MESSAGE("SOURCE_DIR:  "${CMAKE_MODULE_PATH})

set(PVLOG_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

if (CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_CXX_FLAGS "-std=c++0x -Wall -pedantic")
endif (CMAKE_COMPILER_IS_GNUCC)

find_package(sqlite3)

find_Package(GTest)
if (GTEST_FOUND)
	enable_testing()
	add_subdirectory(tests)
endif (GTEST_FOUND)

add_subdirectory(src)


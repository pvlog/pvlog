cmake_minimum_required(VERSION 2.8)

include_directories(${PVLOG_SOURCE_DIR})

if (GTEST_FOUND)
	add_subdirectory(database EXCLUDE_FROM_ALL)
endif (GTEST_FOUND)


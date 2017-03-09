include_directories(util)
include_directories(pvlogdatabase)
include_directories(pvlib/src)
include_directories(pvlog)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(pvlib)
add_subdirectory(pvlog)
add_subdirectory(util)


#if (GTEST_FOUND)
#	add_subdirectory(test EXCLUDE_FROM_ALL)
#endif()

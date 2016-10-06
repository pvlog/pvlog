include_directories(util)
include_directories(pvlogdatabase)
include_directories(pvlib/src)
include_directories(pvlog)


add_subdirectory(pvlib)
add_subdirectory(pvlog)
add_subdirectory(util)
add_subdirectory(ThorSerialize)

#if (GTEST_FOUND)
#	add_subdirectory(test EXCLUDE_FROM_ALL)
#endif()

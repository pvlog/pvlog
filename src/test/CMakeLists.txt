project(test)

include_directories(${PVLOG_SOURCE_DIR})

add_custom_target(check)

macro(run_test test_target)
	add_custom_target(${test_target}_run
	COMMAND ${test_target} 
	DEPENDS ${test_target}
	WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
	add_dependencies(check ${test_target}_run)
endmacro()

#macro(PVLOG_ADD_TESTS TEST_NAME SOURCE)
#	GTEST_ADD_TESTS(TEST_NAME "" ${SOURCE})
#	add_dependencies(check TEST_NAME)
#endmacro(PVLOG_ADD_TESTS TEST_NAME SOURCE)

#
#Database test
#
set(databasesrc databaseTest.cpp)

set(databaselibs pthread ${GTEST_LIBRARIES})

add_executable(databaseTest ${databasesrc})
if (SQLITE3_FOUND)
	set(databaselibs ${databaselibs} ${SQLITE3_LIBRARIES})
else (SQLITE3_FOUND)
	set(databasesrc ${databasesrc} ${PVLOG_SOURCE_DIR}/sqlite3.c)
endif (SQLITE3_FOUND)

set(databaselibs ${databaselibs} pvlogdatabase)

target_link_libraries(databaseTest ${databaselibs})	
run_test(databaseTest)
#add_dependencies(check databaseTest)
#add_test(databaseTestAll databaseTest)
#PVLOG_ADD_TESTS(databaseTest databaseSrc)
#GTEST_ADD_TESTS(databaseTest "" ${databasesrc})
#add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
#	                  DEPENDS databaseTest)

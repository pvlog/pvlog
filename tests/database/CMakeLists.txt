project(DatabaseTest)

set(src databaseTest.cpp)
set(src ${src} ${PVLOG_SOURCE_DIR}/SqliteDatabase.cpp)
set(src ${src} ${PVLOG_SOURCE_DIR}/SqlDatabase.cpp)

add_executable(databaseTest ${src})
if (SQLITE3_FOUND)
	set(libs ${SQLITE3_LIBRARIES})
endif (SQLITE3_FOUND)
target_link_libraries(databaseTest ${libs})	

GTEST_ADD_TESTS(databaseTest "" ${src})


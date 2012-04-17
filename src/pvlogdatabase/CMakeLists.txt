
set(SRC
	SqliteDatabase.cpp
	SqlDatabase.cpp
)

set(HEADERS
	Database.h
	SqlDatabase.h
	SqliteDatabase.h
)

set(LIBS util)

if (SQLITE3_FOUND)
	include_directories(${SQLITE3_INCLUDE_DIRS})
	set(LIBS ${LIBS} ${SQLITE3_LIBRARIES})
else()
	set(SRC ${SRC} sqlite3.c)
	set(LIBS ${LIBS} dl)
	set(HEADERS sqlite3.h)
endif()

add_library(pvlogdatabase ${SRC} ${HEADERS})
target_link_libraries(pvlogdatabase ${LIBS})

project(pvlog)

set(SRC 
	ConfigReader.cpp
	Database.cpp
	Datalogger.cpp
	main.cpp
	Pvlib.cpp
	Pvlog.cpp
	SqlDatabase.cpp
	SqliteDatabase.cpp
	SunriseSunset.cpp
)

set(HEADER
	ConfigReader.h
	Database.h
	SqlDatabase.h
	SqliteDatabase.h
	Datalogger.h
	Pvlib.h
	Pvlog.h
	SunriseSunset.h
	DateTime.h
	PvlogException.h
	Log.h
	ForgroundDaemon.h
	Value.h
	Utility.h
	Daemon.h
)


set(LIBS pthread)
set(LIBS ${LIBS} pvlib)
set(LIBS ${LIBS} bluetooth)
set(LIBS ${LIBS} rt) #clock_gettime

if (SQLITE3_FOUND)
	include_directories(${SQLITE3_INCLUDE_DIRS})
	set(LIBS ${LIBS} ${SQLITE3_LIBRARIES})
else (SQLITE3_FOUND)
	set(SRC ${SRC} sqlite3.c)
	set(LIBS ${LIBS} dl)
endif (SQLITE3_FOUND)

add_executable(pvlog ${SRC} ${HEADER})
target_link_libraries(pvlog ${LIBS})

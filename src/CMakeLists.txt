set(src 
	ConfigReader.cpp
	Database.cpp
	DataLogger.cpp
	main.cpp
	Pvlib.cpp
	Pvlog.cpp
	SqlDatabase.cpp
	SqliteDatabase.cpp
	SunriseSunset.cpp
	Time.cpp)

set(libs pthread)
set(libs ${libs} pvlib)
set(libs ${libs} bluetooth)
set(libs ${libs} rt) #clock_gettime

MESSAGE("SLITE: " ${SQLITE3_FOUND})
if (SQLITE3_FOUND)
	include_directories(${SQLITE3_INCLUDE_DIRS})
	set(libs ${libs} ${SQLITE3_LIBRARIES})
else (SQLITE3_FOUND)
	set(src sqlite3.c)
endif (SQLITE3_FOUND)

add_executable(pvlog ${src})
target_link_libraries(pvlog ${libs})

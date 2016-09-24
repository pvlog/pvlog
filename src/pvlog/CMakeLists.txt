project(pvlog)

set(SRC 
	Datalogger.cpp
	main.cpp
	Pvlib.cpp
	Pvlog.cpp
	SunriseSunset.cpp
)

set(HEADER
	Datalogger.h
	Pvlib.h
	Pvlog.h
	SunriseSunset.h
	ForgroundDaemon.h
	Daemon.h
)

set(ODB_HEADER
	models/Inverter.h
	models/Plant.h
	models/Config.h
	models/SpotData.h
	models/Phase.h
	models/DcInput.h
)

include_directories(../pvlog/models)

set(LIBS util ${ODB_LIBRARIES})


odb_compile(SRC FILES ${ODB_HEADER} DB sqlite GENERATE_QUERY GENERATE_SESSION STANDARD "c++11" DEFAULT_PTR "std::shared_ptr" GENERATE_SCHEMA)


#set(LIBS util pvlogdatabase)
set(LIBS ${LIBS} pthread)
set(LIBS ${LIBS} pvlib)
set(LIBS ${LIBS} bluetooth)
set(LIBS ${LIBS} rt) #clock_gettime
set(LIBS ${LIBS} ${ODB_LIBRARIES})

add_executable(pvlog ${SRC} ${HEADER})
target_link_libraries(pvlog ${LIBS})
target_include_directories(pvlog
	PRIVATE
		${ODB_INCLUDE_DIRS}
		${ODB_COMPILE_OUTPUT_DIR})
target_compile_definitions(pvlog
	PRIVATE
		DATABASE_SQLITE)


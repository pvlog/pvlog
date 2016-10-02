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
	models/DayData.h
)

set(LIBS util ${ODB_LIBRARIES})
set(LIBS ${LIBS} ${Boost_LIBRARIES})
set(LIBS ${LIBS} pthread)
set(LIBS ${LIBS} pvlib)
set(LIBS ${LIBS} bluetooth)
set(LIBS ${LIBS} rt) #clock_gettime

include(${ODB_USE_FILE})
odb_compile(ODB_SRC FILES ${ODB_HEADER} DB sqlite GENERATE_QUERY GENERATE_SESSION
	STANDARD "c++11" DEFAULT_PTR "std::shared_ptr" GENERATE_SCHEMA PROFILE "boost/date-time/gregorian")


include_directories(models)

set(SRC ${SRC} ${ODB_SRC})
add_executable(pvlog ${SRC} ${HEADER})
target_link_libraries(pvlog ${LIBS})
install(TARGETS pvlog DESTINATION bin)
target_include_directories(pvlog
	PRIVATE
		${ODB_INCLUDE_DIRS}
		${ODB_COMPILE_OUTPUT_DIR})
target_compile_definitions(pvlog
	PRIVATE
		DATABASE_SQLITE)

set(SETUP_SRC
	${ODB_SRC}
	setup.cpp
)

add_executable(setup ${SETUP_SRC})
target_link_libraries(setup ${LIBS})
target_include_directories(setup
	PRIVATE
		${ODB_INCLUDE_DIRS}
		${ODB_COMPILE_OUTPUT_DIR})
target_compile_definitions(setup
	PRIVATE
		DATABASE_SQLITE)

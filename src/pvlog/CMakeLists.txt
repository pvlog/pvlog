set(SRC 
	datalogger.cpp
	main.cpp
	sunrisesunset.cpp
	jsonrpcserver.cpp
	pvlibhelper.cpp
	jsonrpcadminserver.cpp
	models/inverter.cpp
	models/plant.cpp
)

set(HEADER
	datalogger.h
	pvlog.h
	sunrisesunset.h
	forgrounddaemon.h
	daemon.h
	abstractpvlogserver.h
	jsonrpcserver.h
)

set(ODB_HEADER
	models/inverter.h
	models/plant.h
	models/config.h
	models/spotdata.h
	models/phase.h
	models/dcinput.h
	models/daydata.h
	models/event.h
)

set(LIBS util ${ODB_LIBRARIES})
set(LIBS ${LIBS} ${Boost_LIBRARIES})
set(LIBS ${LIBS} ${JSONCPP_LIBRARIES})
set(LIBS ${LIBS} ${JSON_RPC_CPP_SERVER_LIBRARIES})
set(LIBS ${LIBS} pthread)
set(LIBS ${LIBS} pvlib)
set(LIBS ${LIBS} bluetooth)
set(LIBS ${LIBS} rt) #clock_gettime

include_directories("${JSONCPP_INCLUDE_DIRS}")
include_directories(models)

get_property(DIRS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)

include(${ODB_USE_FILE})
odb_compile(ODB_SRC FILES ${ODB_HEADER} DB sqlite GENERATE_QUERY GENERATE_SESSION
	STANDARD "c++11" DEFAULT_PTR "std::shared_ptr"
	GENERATE_SCHEMA PROFILE boost/date-time boost/optional
	INCLUDE ${DIRS})



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

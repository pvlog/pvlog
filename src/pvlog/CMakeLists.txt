set(SRC
	datalogger.cpp
	daysummarymessage.cpp
	main.cpp
	email.cpp
	emailnotification.cpp
	messagefilter.cpp
	sunrisesunset.cpp
	jsonrpcserver.cpp
	pvlibhelper.cpp
	jsonrpcadminserver.cpp
	models/inverter.cpp
	models/plant.cpp
)

set(HEADER
	datalogger.h
	email.h
	sunrisesunset.h
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
set(LIBS ${LIBS} ${Poco_LIBRARIES})
set(LIBS ${LIBS} ${PVLIB_LIBRARIES})

include_directories(${ODB_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${JSONCPP_INCLUDE_DIRS})
include_directories(${Poco_INCLUDE_DIRS})
include_directories(${PVLIB_INCLUDE_DIRS})
include_directories(models)

get_property(DIRS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)

include(${ODB_USE_FILE})
odb_compile(ODB_SRC FILES ${ODB_HEADER} DB sqlite GENERATE_QUERY GENERATE_SESSION
	STANDARD "c++11" DEFAULT_PTR "std::shared_ptr"
	GENERATE_SCHEMA SCHEMA_FORMAT "embedded" PROFILE boost/date-time boost/optional
	INCLUDE ${DIRS})



set(SRC ${SRC} ${ODB_SRC})
add_executable(pvlog ${SRC} ${HEADER})
target_link_libraries(pvlog ${LIBS})
target_include_directories(pvlog
	PRIVATE
		${ODB_INCLUDE_DIRS}
		${CMAKE_CURRENT_BINARY_DIR}/odb_gen/)
target_compile_definitions(pvlog
	PRIVATE
		DATABASE_SQLITE)

install(TARGETS pvlog
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

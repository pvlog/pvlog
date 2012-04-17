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


set(LIBS util pvlogdatabase)
set(LIBS ${LIBS} pthread)
set(LIBS ${LIBS} pvlib)
set(LIBS ${LIBS} bluetooth)
set(LIBS ${LIBS} rt) #clock_gettime

add_executable(pvlog ${SRC} ${HEADER})
target_link_libraries(pvlog ${LIBS})

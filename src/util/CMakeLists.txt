set(SRC 
	ConfigReader.cpp
	Log.cpp
	)

set(HEADERS 
	ConfigReader.h 
	DateTime.h 
	Log.h 
	PvlogException.h 
	Utility.h
	)

add_library(util ${SRC} ${HEADERS})

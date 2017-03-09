set(SRC 
	configreader.cpp
	log.cpp
	)

set(HEADERS 
	configreader.h 
	datetime.h 
	log.h 
	pvlogexception.h 
	utility.h
	)

add_library(util ${SRC} ${HEADERS})

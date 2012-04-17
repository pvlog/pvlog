project(pvlogwebCgi)

set(SRC 
	UrlParser.cpp
	DayView.cpp
	main.cpp
)

set(HEADER 
	UrlParser.h
	Log.h
	PvlogwebException.h
	DayView.h
)

add_executable(pvlogweb ${SRC})

project(pvlogweb)

set(CMAKE_CXX_FLAGS "-std=c++0x -Wall -Wwrite-strings -Woverloaded-virtual -Wno-sign-compare")

include_directories(${CMAKE_CURRENT_BINARY_DIR}/ctemplate/src)

add_subdirectory(ctemplate)

set(SRC 
	UrlParser.cpp
	DayView.cpp
	main.cpp
)

set(HEADERS
	UrlParser.h
	DayView.h
)

set(LIBS 
	ctemplate
	util
	pvlogdatabase
)

add_executable(pvlogweb ${SRC})
target_link_libraries(pvlogweb ${LIBS})

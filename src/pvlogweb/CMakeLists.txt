project(pvlogweb)

#set(CMAKE_CXX_FLAGS "-std=c++0x -Wall -Wwrite-strings -Woverloaded-virtual -Wno-sign-compare")
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wwrite-strings -Woverloaded-virtual -Wno-sign-compare")

include_directories(${CMAKE_CURRENT_BINARY_DIR}/ctemplate/src)

add_subdirectory(ctemplate)

set(DATAVIEW_SRC 
	UrlParser.cpp
	DayView.cpp
	main.cpp
)

set(SETUP_SRC
	setup.cpp
	UrlParser.cpp
	Overview.cpp
)

set(DATAVIEW_HEADERS
	UrlParser.h
	DayView.h
)

set(SETUP_HEADERS
	Overview.h
)

set(LIBS 
	ctemplate
	util
	pvlogdatabase
)

add_executable(dataview ${DATAVIEW_SRC} ${DATAVIEW_HEADERS})
target_link_libraries(dataview ${LIBS})

add_executable(setuppvlog ${SETUP_SRC} ${SETUP_HEADERS})
target_link_libraries(setuppvlog ${LIBS})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/templates/dayview.tpl
	${CMAKE_CURRENT_BINARY_DIR}/templates/dayview.tpl)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test/simpleServer.py
	${CMAKE_CURRENT_BINARY_DIR}/test/simpleServer.py)


add_subdirectory(test)

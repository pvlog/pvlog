#project(pvlogweb)
#
##set(CMAKE_CXX_FLAGS "-std=c++0x -Wall -Wwrite-strings -Woverloaded-virtual -Wno-sign-compare")
#set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wwrite-strings -Woverloaded-virtual -Wno-sign-compare")
#
#include_directories(${CMAKE_CURRENT_BINARY_DIR}/ctemplate/src)
#
#add_subdirectory(ctemplate)
#
#set(DATAVIEW_SRC 
#	UrlParser.cpp
#	DayView.cpp
#	main.cpp
#)
#
#set(SETUP_SRC
#	setup.cpp
#	UrlParser.cpp
#	Overview.cpp
#)
#
#set(DATAVIEW_HEADERS
#	UrlParser.h
#	DayView.h
#	PvlogwebException.h
#)
#
#set(SETUP_HEADERS
#	Overview.h
#	PvlogwebException.h
#)
#
#set(LIBS 
#	ctemplate
#	util
#	pvlogdatabase
#)
#
#
#macro (add_cgi_test BINARY)
#	GET_TARGET_PROPERTY(BINARY_LOCATION ${BINARY} LOCATION)
#	# copy bin to binary folder
#	ADD_CUSTOM_COMMAND(TARGET ${BINARY} POST_BUILD 
#		COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/test/cgi
#		COMMAND ${CMAKE_COMMAND} -E copy ${BINARY_LOCATION} ${CMAKE_CURRENT_BINARY_DIR}/test/cgi/)
#endmacro(add_cgi_test)
#
#
#add_executable(dataview ${DATAVIEW_SRC} ${DATAVIEW_HEADERS})
#target_link_libraries(dataview ${LIBS})
#add_cgi_test(dataview)
##ADD_CUSTOM_COMMAND(TARGET dataview POST_BUILD 
##	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/test/cgi)
#
#add_executable(setuppvlog ${SETUP_SRC} ${SETUP_HEADERS})
#target_link_libraries(setuppvlog ${LIBS})
#add_cgi_test(setuppvlog)
#
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/templates/dayview.tpl
#	${CMAKE_CURRENT_BINARY_DIR}/templates/dayview.tpl)
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test/simpleServer.py
#	${CMAKE_CURRENT_BINARY_DIR}/test/simpleServer.py)
#
#
#add_subdirectory(test)

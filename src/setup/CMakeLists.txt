project(pvlog)

set(SRC 
	main.cpp
)

set(LIBS util pvlogdatabase ${Boost_LIBRARIES})

add_executable(setup ${SRC})
target_link_libraries(setup ${LIBS})

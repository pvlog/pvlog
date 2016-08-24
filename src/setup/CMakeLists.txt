set(SRC 
	main.cpp
	../pvlog/SunriseSunset.cpp
)


set(LIBS util pvlogdatabase)

add_executable(setup ${SRC} ${HEADER})
target_link_libraries(setup ${LIBS})

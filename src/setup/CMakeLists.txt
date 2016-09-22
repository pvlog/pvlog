set(SRC 
	main.cpp
)

set(HEADERS
	../pvlog/models/Inverter.h
	../pvlog/models/Plant.h
	../pvlog/models/Config.h
	../pvlog/models/SpotData.h
	../pvlog/models/Phase.h
	../pvlog/models/DcInput.h
)

include_directories(../pvlog)
include_directories(../pvlog/models)

set(LIBS util ${ODB_LIBRARIES})


odb_compile(SRC FILES ${HEADERS} DB sqlite GENERATE_QUERY GENERATE_SESSION STANDARD "c++11" DEFAULT_PTR "std::shared_ptr" GENERATE_SCHEMA)

add_executable(setup ${SRC} ${HEADER})
target_link_libraries(setup ${LIBS})
target_include_directories(setup
	PRIVATE
		${ODB_INCLUDE_DIRS}
		${ODB_COMPILE_OUTPUT_DIR})
target_compile_definitions(setup
	PRIVATE
		DATABASE_SQLITE)

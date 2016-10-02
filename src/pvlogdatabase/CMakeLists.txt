set(ODB_HEADER
	models/Inverter.h
	models/Plant.h
	models/Config.h
	models/SpotData.h
	models/Phase.h
	models/DcInput.h
	models/DayData.h
)

set(LIBS util ${ODB_LIBRARIES})

include(${ODB_USE_FILE})
odb_compile(SRC FILES ${ODB_HEADER} DB sqlite GENERATE_QUERY GENERATE_SESSION 
	STANDARD "c++11" DEFAULT_PTR "std::shared_ptr" GENERATE_SCHEMA PROFILE "boost/date-time/gregorian")


set(LIBS ${LIBS} ${Boost_LIBRARIES})
set(LIBS ${LIBS} ${ODB_LIBRARIES})

include_directories(models)
add_library(pvlogdatabase ${SRC} ${HEADER})
target_link_libraries(pvlogdatabase ${LIBS})
target_include_directories(pvlogdatabase
	PUBLIC
		models
		${ODB_INCLUDE_DIRS}
		${ODB_COMPILE_OUTPUT_DIR})
target_compile_definitions(pvlog
	PUBLIC
		DATABASE_SQLITE)


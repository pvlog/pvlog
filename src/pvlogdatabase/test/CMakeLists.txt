project(test)

set(SRC databaseTest.cpp)

add_unittest(database ${SRC} pvlogdatabase)


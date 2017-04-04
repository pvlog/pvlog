cmake_minimum_required(VERSION 3.0.2)
project(Pvlog)

include(GNUInstallDirs)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(PVLOG_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

if (CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_CXX_FLAGS "-std=c++11 -pthread -Wall -pedantic -Wno-unknown-pragmas -Os")
endif (CMAKE_COMPILER_IS_GNUCC)

find_package(Boost REQUIRED COMPONENTS date_time)
find_package(sqlite3 REQUIRED)
find_package(ODB REQUIRED COMPONENTS sqlite boost)
find_package(Jsoncpp REQUIRED)
find_package(json_rpc_cpp REQUIRED)
find_package(Poco REQUIRED Crypto Net NetSSL_OpenSSL)
find_package(Pvlib REQUIRED)

add_subdirectory(src)

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/pvlog.conf"
       DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/pvlog/)



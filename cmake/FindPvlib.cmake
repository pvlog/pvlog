# Try to find pvlib 
# Once done this will define
#  Pvlib_FOUND - System has pvlib
#  Pvlib_INCLUDE_DIRS - The Pvlib include directories
#  Pvlib_LIBRARIES - The libraries needed to use Pvlib
#  PVLIB_DEFINITIONS - Compiler switches required for using LibXml2

find_package(PkgConfig)
pkg_check_modules(PC_PVLIB QUIET pvlib)
set(PVLIB_DEFINITIONS ${PC_PVLIB_CFLAGS_OTHER})

find_path(PVLIB_INCLUDE_DIR pvlib/pvlib.h
          HINTS ${PC_PVLIB_INCLUDEDIR} ${PC_PVLIB_INCLUDE_DIRS}
	  PATH_SUFFIXES pvlib) 

find_library(PVLIB_LIBRARY NAMES pvlib libpvlib
             HINTS ${PC_PVLIB_LIBDIR} ${PC_PVLIB_LIBRARY_DIRS} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set PVLIB_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Pvlib DEFAULT_MSG
                                  PVLIB_LIBRARY PVLIB_INCLUDE_DIR)

mark_as_advanced(PVLIB_INCLUDE_DIR PVLIB_LIBRARY )

set(PVLIB_LIBRARIES ${PVLIB_LIBRARY} )
set(PVLIB_INCLUDE_DIRS ${PVLIB_INCLUDE_DIR} )

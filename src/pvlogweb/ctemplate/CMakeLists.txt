cmake_minimum_required(VERSION 2.8)

project(ctemplate C CXX)

include(CheckIncludeFiles)
include(CheckTypeSize)
include(CheckFunctionExists)
include(FindThreads)

set(SO_VERSION "2:0:0")

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CXXFLAGS "-std=c++0x -Wall -Wwrite-strings -Woverloaded-virtual -Wno-sign-compare")
endif()

set(GOOGLE_NAMESPACE "::ctemplate")
set(HTMLPARSER_NAMESPACE google_ctemplate_streamhtmlparser)
set(START_GOOGLE_NAMESPACE "namespace ctemplate {")
set(END_GOOGLE_NAMESPACE "}")

macro(normalize_boolean BOOL_IN)
	if (${BOOL_IN})
		set(${BOOL_IN} 1)
	else()
		set(${BOOL_IN} 0)
	endif()
endmacro()

set(CMAKE_CXX_FLAGS ${CXXFLAGS})

set(CMAKE_HELPERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

check_include_files(byteswap.h HAVE_BYTESWAP_H)
check_include_files(dirent.h HAVE_DIRENT_H)
check_include_files(dlfcn.h HAVE_DLFCN_H)
check_include_files(endian.h HAVE_ENDIAN_H)
check_include_files(getopt.h HAVE_GETOP_H)
check_function_exists(getopt HAVE_GETOPT)
check_function_exists(getopt_long HAVE_GETOPT_LONG)
check_include_files(libkern/OSByteOrder.h HAVE_LIBKERN_OSBYTEORDER)
check_include_files(machine/endian.h HAVE_MACHINE_ENDIAN_H)
check_include_files(memory.h HAVE_MEMORY_H)
check_include_files(ndir.h HAVE_NDIR_H)
check_include_files(sys/byteorder.h HAVE_SYS_BYTEORDER_H)
check_include_files(sys/dir.h HAVE_SYS_DIR_H)
check_include_files(sys/endian.h HAVE_SYS_ENDIAN_H)
check_include_files(sys/isa_defs.h HAVE_SYS_ISA_DEFS_H)
check_include_files(sys/ndir.h HAVE_SYS_NDIR_H)
check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(sys/unistd.h HAVE_UNISTD_H)
check_include_files(utime.h HAVE_UTIME_H)

check_include_files(inttypes.h HAVE_INTTYPES_H)
check_include_files(stdint.h HAVE_STDINT_H)

#Find unordered map and unordered map
#try_compile(GCC_4_2 ${PROJECT_BINARY_DIR} ${CMAKE_HELPERS_DIR}/gcc_version_min_4_2.cc)
set(HASH_MAP_H "<unordered_map>")
set(HASH_MAP_CLASS std::unordered_map)
set(HASH_SET_H "<unordered_set>")
set(HASH_SET_CLASS std::unordered_set)
set(HASH_NAMESPACE std)
set(HAVE_UNORDERED_MAP 1) #we have unordered_{map,set}
#set(HAVE_HASH_MAP 1)
#set(HAVE_HASH_SET 1)

if (NOT DEFINED HAVE_INTTYPES AND NOT DEFINED HAVE_STDINT_H)
	check_type_size(__int64_t __INT64_T)
	check_type_size("long long" LONG_LONG)
	check_type_size(size_t SIZE_T)
	check_type_size(int INT)

	if (HAVE___INT64_T) 
		set(UINT64_T "unsigned __int64")
	elseif (${LONG} equal 8)
		set(UINT64_T "unsigned long")
	elseif (${LONG_LONG} equal 8) 
		set(UINT64_T "unsigned long long")
	else()
       		message(FATAL_ERROR "ERROR: could not find 64 bit width integer datatype")
	endif()

	if (${SIZE_T} EQUAL ${INT})
		set(PRIUS_PREFIX "")
	elseif (${SIZE_T} EQUAL ${LONG})
		set(PRIUS_PREFIX "l")
	endif()

	set(HAVE_UINT64_T 1)
	set(HAVE_UINT64_T 1)
	set(UINT32_T "unsigned int")

else() 
	set(UINT64_T uint64_t)
	set(HAVE_UINT64_T 1)
	set(HAVE_UINT32_T 1)
	set(PRIUS_PREFIX z)
endif()


set(PRIdS ${PRIUS_PREFIX}d)
set(PRIuS ${PRIUS_PREFIX}u)
set(PRIxS ${PRIUS_PREFIX}x)



#Check if compiler supports __ATTRIBUTE__
try_compile(HAVE___ATTRIBUTE__ ${PROJECT_BINARY_DIR} ${CMAKE_HELPERS_DIR}/attribute.cc)
message("try compile HEAVE___ATTRIBUTE__: " ${HAVE___ATTRIBUTE__})

# One some systems (eg gnu/linux), the linker defines _start and
# data_start to indicate the extent of the .text section.  We can use
# this to know strings are immutable.  In the code, we make the
# variables weak, just in case, but for this check, we only want to
# say "yes" if the linker supports the vars, *and* the compiler supports
# attribute-weak.
try_compile(HAVE_ATTRIBUTE_WEAK ${PROJECT_BINARY_DIR} ${CMAKE_HELPERS_DIR}/attribute_weak.cc)
message("try compile HAVE_ATTRIBUTE_WEAK: " ${HAVE_ATTRIBUTE_WEAK})

find_package(Threads)
if (DEFINED CMAKE_USE_PTHREADS_INIT)
	set(HAVE_PTHREAD 1)
	# Note: Cygwin (as of 12/1/08) has a bug in pthread_rwlock, where
	# if you try to acquire a read-lock twice, you get EDEADLCK, where
	# it should succeed.  It would be nice to test for that, but we
	# can't do runtime checks here.  So we just manually fail for CYGWIN.
	if (NOT DEFINED CYGWIN) 
		try_compile(HAVE_RWLOCK ${PROJECT_BINARY_DIR} 
			${CMAKE_HELPERS_DIR}/rwlock.cc
			CMAKE_FLAGS "-DLINK_LIBRARIES=${CMAKE_THREAD_LIBS_INIT}")
		message("try compile: HAVE_RWLOCK: " ${HAVE_RWLOCK})
	endif()
endif ()

if (WIN32) 
	try_compile(INTERLOCKED_EXCHANGE_NONVOLATILE ${CMAKE_HELPERS_DIR}/interlocked_exchange.cc)
	message("try compile: INTERLOCKED_EXCHANGE_NONVOLATILE" ${INTERLOCKED_EXCHANGE_NONVOLATILE})
endif (WIN32)

normalize_boolean(HAVE___ATTRIBUTE__)
normalize_boolean(HAVE_ATTRIBUTE_WEAK)

set(_START_GOOGLE_NAMESPACE_ ${START_GOOGLE_NAMESPACE})
set(_END_GOOGLE_NAMESPACE_ ${END_GOOGLE_NAMESPACE})

set(ac_google_namespace ${GOOGLE_NAMESPACE})
set(ac_googlr_start_namespace ${START_GOOGLE_NAMESPACE})
set(ac_google_namespace ${GOOGLE_NAMESPACE})
set(ac_google_start_namespace ${START_GOOGLE_NAMESPACE})
set(ac_google_end_namespace ${END_GOOGLE_NAMESPACE})
set(ac_cv_cxx_hash_map ${HASH_MAP_H})
set(ac_cv_cxx_hash_set ${HASH_SET_H})
set(ac_cv_cxx_hash_map_class ${HASH_MAP_CLASS})
set(ac_cv_cxx_hash_set_class ${HASH_SET_CLASS})
set(ac_google_attribute ${HAVE___ATTRIBUTE__})
set(ac_cv_uint64 ${UINT64_T})
set(ac_cv_have_stdint_h ${HAVE_STDINT_H})
set(ac_cv_have_inttypes_h ${HAVE_INTTYPES_H})
set(ac_have_attribute_weak ${HAVE_ATTRIBUTE_WEAK})
#
# windows specific
#
set(ac_windows_dllexport_defines ${WINDOWS_DLLEXPORT_DEFINES})
set(ac_windows_dllexport ${WINDOWS_DLLEXPORT})
#
#end windows specific
set(ac_htmlparser_namespace ${HTMLPARSER_NAMESPACE})



set(HEADERS_IN ${CMAKE_CURRENT_SOURCE_DIR}/src/ctemplate)
set(HEADERS_OUT ${CMAKE_CURRENT_BINARY_DIR}/src/ctemplate)

configure_file(${HEADERS_IN}/template_string.h.in ${HEADERS_OUT}/template_string.h)
configure_file(${HEADERS_IN}/template_enums.h.in ${HEADERS_OUT}/template_enums.h)
configure_file(${HEADERS_IN}/template.h.in ${HEADERS_OUT}/template.h)
configure_file(${HEADERS_IN}/template_cache.h.in ${HEADERS_OUT}/template_cache.h)
configure_file(${HEADERS_IN}/template_modifiers.h.in ${HEADERS_OUT}/template_modifiers.h)
configure_file(${HEADERS_IN}/template_emitter.h.in ${HEADERS_OUT}/template_emitter.h)
configure_file(${HEADERS_IN}/template_annotator.h.in ${HEADERS_OUT}/template_annotator.h)
configure_file(${HEADERS_IN}/template_dictionary.h.in ${HEADERS_OUT}/template_dictionary.h)
configure_file(${HEADERS_IN}/template_dictionary_interface.h.in 
	${HEADERS_OUT}/template_dictionary_interface.h)
configure_file(${HEADERS_IN}/template_pathops.h.in ${HEADERS_OUT}/template_pathops.h)
configure_file(${HEADERS_IN}/template_namelist.h.in ${HEADERS_OUT}/template_namelist.h)
configure_file(${HEADERS_IN}/find_ptr.h.in ${HEADERS_OUT}/find_ptr.h)
configure_file(${HEADERS_IN}/per_expand_data.h.in ${HEADERS_OUT}/per_expand_data.h)
configure_file(${HEADERS_IN}/str_ref.h.in ${HEADERS_OUT}/str_ref.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/src/config.h)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(CTEMPLATEINCLUDE_HEADERS_DIR ${SOURCE_DIR}/ctemplate)

include_directories(${CTEMPLATEINCLUDE_HEADERS_DIR})
include_directories(${SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src)

set(CTEMPLATEINCLUDE_HEADERS
	${CTEMPLATEINCLUDE_HEADERS_DIR}/template.h
	${CTEMPLATEINCLUDE_HEADERS_DIR}/template_cache.h
	${CTEMPLATEINCLUDE_HEADERS_DIR}/template_string.h
	${CTEMPLATEINCLUDE_HEADERS_DIR}/template_enums.h
	${CTEMPLATEINCLUDE_HEADERS_DIR}/template_pathops.h
	${CTEMPLATEINCLUDE_HEADERS_DIR}/template_modifiers.h
	${CTEMPLATEINCLUDE_HEADERS_DIR}/template_dictonary.h
	${CTEMPLATEINCLUDE_HEADERS_DIR}/template_dictionary_interface.h
	${CTEMPLATEINCLUDE_HEADERS_DIR}/template_annotator.h
	${CTEMPLATEINCLUDE_HEADERS_DIR}/template_emitter.h
	${CTEMPLATEINCLUDE_HEADERS_DIR}/template_namelist.h
	${CTEMPLATEINCLUDE_HEADERS_DIR}/per_expand_data.h
	${CTEMPLATEINCLUDE_HEADERS_DIR}/str_ref.h
)

set(LIBCTEMPLATE_LA_SOURCES
	${SOURCE_DIR}/base/arena-inl.h
	${SOURCE_DIR}/base/arena.cc
	${SOURCE_DIR}/base/fileutil.h
	${SOURCE_DIR}/base/macros.h
	${SOURCE_DIR}/base/manual_constructor.h
	${SOURCE_DIR}/base/mutex.h
	${SOURCE_DIR}/base/small_map.h
	${SOURCE_DIR}/base/thread_annotations.h
	${SOURCE_DIR}/base/util.h
	${SOURCE_DIR}/indented_writer.h
	${SOURCE_DIR}/per_expand_data.cc
	${SOURCE_DIR}/template.cc
	${SOURCE_DIR}/template_annotator.cc
	${SOURCE_DIR}/template_cache.cc
	${SOURCE_DIR}/template_dictionary.cc
	${SOURCE_DIR}/template_modifiers.cc
	${SOURCE_DIR}/template_modifiers_internal.h
	${SOURCE_DIR}/template_namelist.cc
	${SOURCE_DIR}/template_pathops.cc
	${SOURCE_DIR}/template_string.cc
	${SOURCE_DIR}/htmlparser/htmlparser.cc
	${SOURCE_DIR}/htmlparser/htmlparser.h
	${SOURCE_DIR}/htmlparser/htmlparser_cpp.h
	${SOURCE_DIR}/htmlparser/jsparser.cc
	${SOURCE_DIR}/htmlparser/jsparser.h
	${SOURCE_DIR}/htmlparser/statemachine.cc
	${SOURCE_DIR}/htmlparser/statemachine.h
)

#set(CTEMPLATE_SYMBOLS "(ctemplate|template_modifiers|BaseArena|UnsafeArena|[^A-Za-z]Template[^A-Za-z]|TemplateDictionary|TemplateNamelist|TemplateFromString|TemplateString|TemplateState|[^A-Za-z]Strip[^A-Za-z]a)")


#set(LIBCTEMPLATE_LA_LDFLAGS "-export-symbol-regex ${CTEMPLATE_SYMBOLS} -version-info ${SO_VERSION}")

add_library(ctemplate STATIC ${LIBCTEMPLATE_LA_SOURCES})
#set_target_properties(ctemplate PROPERTIES STATIC_LIBRARY_FLAGS ${LIBCTEMPLATE_LA_LDFLAGS})
target_link_libraries(ctemplate ${CMAKE_THREAD_LIBS_INIT})
message("threads: " ${CMAKE_THREAD_LIBS_INIT})

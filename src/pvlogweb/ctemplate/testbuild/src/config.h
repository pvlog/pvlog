#ifndef CONFIG_H
#define CONFIG_H

/* Namespace for Google classes */
#define GOOGLE_NAMESPACE ::ctemplate

/* the location of <unordered_map> or <hash_map> */
#define HASH_MAP_H <unordered_map>

/* the namespace of hash_map/hash_set */
#define HASH_NAMESPACE std

/* the location of <unordered_set> or <hash_set> */
#define HASH_SET_H <unordered_set>

/* Define to 1 if you have the <byteswap.h> header file. */
#define HAVE_BYTESWAP_H 1

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'. */
#define HAVE_DIRENT_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <endian.h> header file. */
#define HAVE_ENDIAN_H 1

/* Define to 1 if you have the <getopt.h> header file. */
#define HAVE_GETOP_H 1

/* Define to 1 if you have the `getopt' function. */
/* #undef HAVE_GETOP */

/* Define to 1 if you have the `getopt_long' function. */
#define HAVE_GETOPT_LONG 1

/* define if the compiler has hash_map */
/* #undef HAVE_HASH_MAP */

/* define if the compiler has hash_set */
/* #undef HAVE_HASH_SET */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <libkern/OSByteOrder.h> header file. */
/* #undef HAVE_LIBKERN_OSBYTEORDER_H */

/* Define to 1 if you have the <machine/endian.h> header file. */
/* #undef HAVE_MACHINE_ENDIAN_H */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* define if the compiler implements namespaces */
//#undef HAVE_NAMESPACES

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define if you have POSIX threads libraries and header files. */
#define HAVE_PTHREAD 1

/* define if the compiler implements pthread_rwlock_* */
#define HAVE_RWLOCK 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
//#undef HAVE_STDLIB_H

/* Define to 1 if you have the <strings.h> header file. */
//#undef HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
//#undef HAVE_STRING_H


/* Define to 1 if you have the <sys/byteorder.h> header file. */
/* #undef HAVE_SYS_BYTEORDER_H */

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'. */
#define HAVE_SYS_DIR_H 1

/* Define to 1 if you have the <sys/endian.h> header file. */
/* #undef HAVE_SYS_ENDIAN_H */

/* Define to 1 if you have the <sys/isa_defs.h> header file. */
/* #undef HAVE_SYS_ISA_DEFS_H */

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if the system has the type `uint32_t'. */
#define HAVE_UINT32_T

/* Define to 1 if the system has the type `uint64_t'. */
#define HAVE_UINT64_T

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H

/* define if the compiler supports unordered_{map,set} */
#define HAVE_UNORDERED_MAP 1

/* Define to 1 if you have the <utime.h> header file. */
#define HAVE_UTIME_H 1

/* Define to 1 if the system has the type `u_int32_t'. */
//#undef HAVE_U_INT32_T

/* Define to 1 if the system has the type `u_int64_t'. */
//#undef HAVE_U_INT64_T

/* define if your compiler has __attribute__ */
#define HAVE___ATTRIBUTE__

/* Define to 1 if the system has the type `__int32'. */
//#undef HAVE___INT32

/* Define to 1 if the system has the type `__int64'. */
//#undef HAVE___INT64

/* The namespace to put the htmlparser code. */
#define HTMLPARSER_NAMESPACE google_ctemplate_streamhtmlparser

/* define if first argument to InterlockedExchange is just LONG */
/* #undef INTERLOCKED_EXCHANGE_NONVOLATILE */

/* Define to the sub-directory in which libtool stores uninstalled libraries.
      */
//#undef LT_OBJDIR

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
//#undef NO_MINUS_C_MINUS_O

/* Name of package */
//#undef PACKAGE

/* Define to the address where bug reports for this package should be sent. */
//#undef PACKAGE_BUGREPORT

/* Define to the full name of this package. */
//#undef PACKAGE_NAME

/* Define to the full name and version of this package. */
//#undef PACKAGE_STRING

/* Define to the one symbol short name of this package. */
//#undef PACKAGE_TARNAME

/* Define to the home page for this package. */
//#undef PACKAGE_URL

/* Define to the version of this package. */
//#undef PACKAGE_VERSION

/* printf format code for printing a size_t and ssize_t */
#define PRIdS "ld"

/* printf format code for printing a size_t and ssize_t */
#define PRIuS "lu"

/* printf format code for printing a size_t and ssize_t */
/* #undef PPRIxS */

/* Define to necessary symbol if this constant uses a non-standard name on your system. */
//#undef PTHREAD_CREATE_JOINABLE

/* the namespace where STL code like vector<> is defined */
#define STL_NAMESPACE std

/* Version number of package */
#define version "2.1"

/* Stops putting the code inside the Google namespace */
#define _END_GOOGLE_NAMESPACE_ }

/* Puts following code inside the Google namespace */
#define _START_GOOGLE_NAMESPACE_ namespace ctemplate {

#if defined( __MINGW32__) || defined(__MINGW64__)
#include "windows/port.h"
#endif


#endif /* #ifndef CONFIG_H */

s/@ac_google_namespace@/${GOOGLE_NAMESPACE}/g;
s/@ac_google_start_namespace@/${START_GOOGLE_NAMESPACE}/g;
s/@ac_google_end_namespace@/${END_GOOGLE_NAMESPACE}/g;
s/@ac_cv_cxx_hash_map@/${HASH_MAP_H}/g;
s/@ac_cv_cxx_hash_set@/${HASH_SET_H}/g;
s/@ac_cv_cxx_hash_map_class@/${HASH_MAP_CLASS$}/g;
s/@ac_cv_cxx_hash_set_class@/${HASH_SET_CLASS}/g;
s/@ac_google_attribute@/${HAVE___ATTRIBUTE___}/g;
s/@ac_cv_uint64@/${UINT64}/g;
s/@ac_cv_have_stdint_h@/${HAVE_STDINT_H}/g;
s/@ac_cv_have_inttypes_h@/${HAVE_INTTYPES_H}/g;
s/@ac_have_attribute_weak@/${HAVE_ATTRIBUTE_WEAK}/g;
#
# windows specific
#
s/@ac_windows_dllexport_defines@/${WINDOWS_DLLEXPORT_DEFINES}/g;
s/@ac_windows_dllexport@/${WINDOWS_DLLEXPORT}/g;
#
#end windows specific
s/@ac_htmlparser_namespace@/${HTMLPARSER_NAMESPACE}/g;

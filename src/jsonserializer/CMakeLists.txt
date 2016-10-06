set(SRC 
	JsonParser.cpp
	JsonPrinter.cpp
	Serialize.cpp
	Serialize.tpp
)

set(HEADERS 
	Exporter.h 
	Importer.h 
	JsonLexemes.h 
	JsonLexer.h 
	JsonParser.h
	JsonPrinter.h
	JsonThor.h
	Serialize.h
	Traits.h
	UnicodeIterator.h
)

find_package(FLEX REQUIRED)
FLEX_TARGET(MyScanner JsonLexer.l  ${CMAKE_CURRENT_BINARY_DIR}/JsonLexer.lex.cpp
	COMPILE_FLAGS "--header-file=${CMAKE_CURRENT_BINARY_DIR}/JsonLexer.lex.h")

add_library(thorsserializer ${SRC} ${HEADERS} ${FLEX_MyScanner_OUTPUTS})

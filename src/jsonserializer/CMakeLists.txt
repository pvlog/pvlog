set(SRC 
	JsonLexer.lex.cpp
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
	JsonLexer.lex.h
	JsonParser.h
	JsonPrinter.h
	JsonThor.h
	Serialize.h
	Traits.h
	UnicodeIterator.h
	)

add_library(thorsserializer ${SRC} ${HEADERS})

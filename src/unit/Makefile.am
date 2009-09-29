check_PROGRAMS = test_json_parser

check:
	./test_json_parser

test_json_parser_SOURCES = test_json_parser.hh test_json_parser.cc main.cc \
	../json_parser.hh \
	../json_scanner.hh \
	../json_scanner.cc \
	../json_tree.hh \
	../json_tree.cc

test_json_parser_CXXFLAGS = -I $(top_srcdir)/src `cppunit-config --cflags`
test_json_parser_LDFLAGS = `cppunit-config --libs`

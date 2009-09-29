#include <cppunit/TestAssert.h>
#include <cstring>
#include <cwchar>
#include "test_json_parser.hh"

CPPUNIT_TEST_SUITE_REGISTRATION(TestJSONParser<char>);
//CPPUNIT_TEST_SUITE_REGISTRATION(TestJSONParser<wchar_t>);

template<>
size_t
strlen<char>(const char *s) {
	return strlen(s);
}

template<>
size_t
strlen<wchar_t>(const wchar_t *s) {
	return wcslen(s);
}



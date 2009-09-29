#ifndef __TEST_JSON_PARSER_HH__
#define __TEST_JSON_PARSER_HH__

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "json_parser.hh"
#include "json_scanner.hh"
#include "json_tree.hh"

template<typename Char> size_t strlen(const Char *);

template<typename Char>
class TestJSONParser : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(TestJSONParser<Char>); // name of the test fixture class

	CPPUNIT_TEST(ok_in_one_no_whitespace);
	CPPUNIT_TEST(ok_in_one_trailing_whitespace);
	CPPUNIT_TEST(ok_in_one_init_whitespace);
	CPPUNIT_TEST(ok_multi_no_whitespace);
	CPPUNIT_TEST(ok_multi_trailing_whitespace);
	CPPUNIT_TEST(ok_multi_whitespace_chunk);
	CPPUNIT_TEST(ok_multi_incomplete_token_chunk);
	CPPUNIT_TEST(error_trailing_junk);
	CPPUNIT_TEST(error_premature_eos);
	CPPUNIT_TEST(error_syntax_error_at_beginning_of_chunk);
	CPPUNIT_TEST(error_syntax_error_within_chunk);
	CPPUNIT_TEST(error_scanner_error_at_beginning_of_chunk);
	CPPUNIT_TEST(error_scanner_error_within_chunk);
	CPPUNIT_TEST(error_trailing_incomplete_token);

	CPPUNIT_TEST_SUITE_END();

	void ok_in_one_no_whitespace();
	void ok_in_one_trailing_whitespace();
	void ok_in_one_init_whitespace();
	void ok_multi_no_whitespace();
	void ok_multi_trailing_whitespace();
	void ok_multi_whitespace_chunk();
	void ok_multi_incomplete_token_chunk();
	void error_trailing_junk();
	void error_premature_eos();
	void error_syntax_error_at_beginning_of_chunk();
	void error_syntax_error_within_chunk();
	void error_scanner_error_at_beginning_of_chunk();
	void error_scanner_error_within_chunk();
	void error_trailing_incomplete_token();
public:
	void setUp();
	void tearDown();
};

template<typename Char>
void
TestJSONParser<Char>::setUp() {
}

template<typename Char>
void
TestJSONParser<Char>::tearDown() {
}

template<typename Char>
void
TestJSONParser<Char>::ok_in_one_no_whitespace() {
	const Char *chunks[] = {
		"{ \"h\\u00eq\\\"\\t\\n\\r\\f\\u0043\\uc3a9\\\\\\u00e9e\\b\\/a\\\"\\xa\\u12\" : +1.3e+1 }",
		"",
		""
	};

	std::basic_istringstream<Char> s;	
	json::parser<Char> parser(s);
	std::stack<json::internal_node *> ctx;
	parser.hook_obj_start(reinterpret_cast<typename json::parser<Char>::hook_start_end_t>(&obj_start_cb));
	parser.hook_key(reinterpret_cast<typename json::parser<Char>::hook_key_t>(&key_cb));
	parser.hook_obj_data(reinterpret_cast<typename json::parser<Char>::hook_primitive_t>(&obj_data_cb));
	parser.hook_obj_end(reinterpret_cast<typename json::parser<Char>::hook_start_end_t>(&obj_end_cb));
	parser.hook_array_start(reinterpret_cast<typename json::parser<Char>::hook_start_end_t>(&array_start_cb));
	parser.hook_array_data(reinterpret_cast<typename json::parser<Char>::hook_primitive_t>(&array_data_cb));
	parser.hook_array_end(reinterpret_cast<typename json::parser<Char>::hook_start_end_t>(&array_end_cb));
	parser.set_context(&ctx);
	json::root_node *r = new json::root_node();
	ctx.push(r);

	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[0]), strlen(chunks[0]));
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[1]), strlen(chunks[1]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[2]), strlen(chunks[2]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::OK == parser.parse());
	
	CPPUNIT_ASSERT(1 == ctx.size());
	json::root_node *n = reinterpret_cast<typename json::root_node *>(ctx.top());
	CPPUNIT_ASSERT(0 != n);
	std::cerr << *n << std::endl;
	delete n;
}

template<typename Char>
void
TestJSONParser<Char>::ok_in_one_trailing_whitespace() {
	const Char *chunks[] = {
		"{ \"h\\\"\\\\e\\/a\\\"a\" : 1.3e+1, \"obj\" : {}, \"xi\" : {\"phi\" : \"omega\"}, \"\" : [null, true, false], \"null\" : [true], \"dolly\" : [], \"a\" : 0, \"b\" : 0., \"c\" : 0.0, \"d\" : 1e-1, \"e\" : [\"done\"], \"f\" : \"ok\", \"g\" : [{\"h\" : 2, \"i\" : null, \"j\" : false, \"k\" : true}, null, {}, .8]} ",
		""
	};

	std::basic_istringstream<Char> s;	
	json::parser<Char> parser(s);
	std::stack<json::internal_node *> ctx;
	parser.hook_obj_start(reinterpret_cast<typename json::parser<Char>::hook_start_end_t>(&obj_start_cb));
	parser.hook_key(reinterpret_cast<typename json::parser<Char>::hook_key_t>(&key_cb));
	parser.hook_obj_data(reinterpret_cast<typename json::parser<Char>::hook_primitive_t>(&obj_data_cb));
	parser.hook_obj_end(reinterpret_cast<typename json::parser<Char>::hook_start_end_t>(&obj_end_cb));
	parser.hook_array_start(reinterpret_cast<typename json::parser<Char>::hook_start_end_t>(&array_start_cb));
	parser.hook_array_data(reinterpret_cast<typename json::parser<Char>::hook_primitive_t>(&array_data_cb));
	parser.hook_array_end(reinterpret_cast<typename json::parser<Char>::hook_start_end_t>(&array_end_cb));
	parser.set_context(&ctx);
	json::root_node *r = new json::root_node();
	ctx.push(r);

	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[0]), strlen(chunks[0]));
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[1]), strlen(chunks[1]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::OK == parser.parse());

	CPPUNIT_ASSERT(1 == ctx.size());
	json::root_node *n = reinterpret_cast<typename json::root_node *>(ctx.top());
	CPPUNIT_ASSERT(0 != n);
	std::cerr << *n << std::endl;
	delete n;
}

template<typename Char>
void
TestJSONParser<Char>::ok_in_one_init_whitespace() {
	const Char *chunks[] = {
		"   { \"h\\\"\\\\e\\/a\\\"a\" : -1.3e+1 }",
		"",
		""
	};

	std::basic_istringstream<Char> s;	
	json::parser<Char> parser(s);
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[0]), strlen(chunks[0]));
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[1]), strlen(chunks[1]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[2]), strlen(chunks[2]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::OK == parser.parse());
}

template<typename Char>
void
TestJSONParser<Char>::ok_multi_no_whitespace() {
	const Char *chunks[] = {
		"{ \"h\\\"\\\\e\\/a\\",
		"\"a\" : 1.3",
		"e+1 }",
		"",
		""
	};

	std::basic_istringstream<Char> s;	
	json::parser<Char> parser(s);
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[0]), strlen(chunks[0]));
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[1]), strlen(chunks[1]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[2]), strlen(chunks[2]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[3]), strlen(chunks[3]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[4]), strlen(chunks[4]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::OK == parser.parse());
}

template<typename Char>
void
TestJSONParser<Char>::ok_multi_trailing_whitespace() {
	const Char *chunks[] = {
		"{ \"h\\\"\\\\e\\/a\\",
		"\"a\" : 1.3",
		"e+1 } ",
		""
	};

	std::basic_istringstream<Char> s;	
	json::parser<Char> parser(s);
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[0]), strlen(chunks[0]));
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[1]), strlen(chunks[1]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[2]), strlen(chunks[2]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[3]), strlen(chunks[3]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::OK == parser.parse());
}

template<typename Char>
void
TestJSONParser<Char>::ok_multi_whitespace_chunk() {
	const Char *chunks[] = {
		"{ \"h\\\"\\\\e\\/a\\",
		"\"a\" : 1.3",
		"    ",
		" ",
		"}",
		"",
		""
	};

	std::basic_istringstream<Char> s;	
	json::parser<Char> parser(s);
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[0]), strlen(chunks[0]));
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[1]), strlen(chunks[1]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[2]), strlen(chunks[2]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[3]), strlen(chunks[3]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[4]), strlen(chunks[4]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[5]), strlen(chunks[5]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[6]), strlen(chunks[6]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::OK == parser.parse());
}

template<typename Char>
void
TestJSONParser<Char>::ok_multi_incomplete_token_chunk() {
	const Char *chunks[] = {
		"{ \"h\\\"\\\\e\\/a\\",
		"\"a\" : 1.",
		"e+1",
		" ",
		"}",
		"",
		""
	};

	std::basic_istringstream<Char> s;	
	json::parser<Char> parser(s);
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[0]), strlen(chunks[0]));
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[1]), strlen(chunks[1]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[2]), strlen(chunks[2]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[3]), strlen(chunks[3]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[4]), strlen(chunks[4]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[5]), strlen(chunks[5]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[6]), strlen(chunks[6]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::OK == parser.parse());
}

template<typename Char>
void
TestJSONParser<Char>::error_trailing_junk() {
	const Char *chunks[] = {
		"{ \"h\\\"\\\\e\\/a\\",
		"\"a\" : .3",
		"e+1",
		" ",
		"}",
		"false",
		""
	};

	std::basic_istringstream<Char> s;	
	json::parser<Char> parser(s);
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[0]), strlen(chunks[0]));
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[1]), strlen(chunks[1]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[2]), strlen(chunks[2]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[3]), strlen(chunks[3]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[4]), strlen(chunks[4]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[5]), strlen(chunks[5]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[6]), strlen(chunks[6]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::ERROR == parser.parse());
}

template<typename Char>
void
TestJSONParser<Char>::error_premature_eos() {
	const Char *chunks[] = {
		"{ \"h\\\"\\\\e\\/a\\",
		"\"a\" : 0.3",
		"e+1",
		" ",
		""
	};

	std::basic_istringstream<Char> s;	
	json::parser<Char> parser(s);
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[0]), strlen(chunks[0]));
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[1]), strlen(chunks[1]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[2]), strlen(chunks[2]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[3]), strlen(chunks[3]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[4]), strlen(chunks[4]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::ERROR == parser.parse());
}

template<typename Char>
void
TestJSONParser<Char>::error_syntax_error_at_beginning_of_chunk() {
	const Char *chunks[] = {
		"{ fals",
		"e : 1.3"
	};

	std::basic_istringstream<Char> s;	
	json::parser<Char> parser(s);
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[0]), strlen(chunks[0]));
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[1]), strlen(chunks[1]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::ERROR == parser.parse());
}

template<typename Char>
void
TestJSONParser<Char>::error_syntax_error_within_chunk() {
	const Char *chunks[] = {
		"{ \"h\\\"\\\\e\\/a\\",
		"\"a\" , 1.3"
	};

	std::basic_istringstream<Char> s;	
	json::parser<Char> parser(s);
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[0]), strlen(chunks[0]));
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[1]), strlen(chunks[1]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::ERROR == parser.parse());
}

template<typename Char>
void
TestJSONParser<Char>::error_scanner_error_at_beginning_of_chunk() {
	const Char *chunks[] = {
		"tri"
	};

	std::basic_istringstream<Char> s;	
	json::parser<Char> parser(s);
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[0]), strlen(chunks[0]));
	CPPUNIT_ASSERT(json::parser<Char>::ERROR == parser.parse());
}

template<typename Char>
void
TestJSONParser<Char>::error_scanner_error_within_chunk() {
	const Char *chunks[] = {
		"{ \"h\\\"\\\\e\\/a\\",
		"\"a\" : tri"
	};

	std::basic_istringstream<Char> s;	
	json::parser<Char> parser(s);
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[0]), strlen(chunks[0]));
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[1]), strlen(chunks[1]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::ERROR == parser.parse());
}

template<typename Char>
void
TestJSONParser<Char>::error_trailing_incomplete_token() {
	const Char *chunks[] = {
		"{ \"h\\\"\\\\e\\/a\\",
		"\"a\" : -.3",
		"e+1",
		" ",
		"}",
		"fal",
		""
	};

	std::basic_istringstream<Char> s;	
	json::parser<Char> parser(s);
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[0]), strlen(chunks[0]));
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[1]), strlen(chunks[1]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[2]), strlen(chunks[2]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[3]), strlen(chunks[3]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[4]), strlen(chunks[4]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[5]), strlen(chunks[5]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::PENDING == parser.parse());
	s.rdbuf()->pubsetbuf(const_cast<Char *>(chunks[6]), strlen(chunks[6]));
	s.clear();
	CPPUNIT_ASSERT(json::parser<Char>::ERROR == parser.parse());
}

#endif

#include <sstream>
#include <cstring>
#include <iostream>
#include <stack>
#include "json_parser.hh"
#include "json_tree.hh"

int
main(int argc, char *argv[]) {
	const char *chunks[] = {
		"[ \"h\\\"\\u00eq\\\\e\\/a\\\"a\" , 1.3e+1, \"obj\" ,",
		" {}, \"xi\" , {\"phi\" : \"omega\\u12\"}, \"\" , [",
		"null, true, false], \"null\" , [true], \"doll",
		"y\" , [], \"a\" , 0, \"b\" , 0., \"c\" , 0.0,",
		" \"d\" , 1e-1, \"e\", [\"done\"], \"f\" , \"",
		"ok\", \"g\" , [{\"h\\u00e9\" : 2, \"i\" : null, \"j",
		"\" : false, \"k\" : true}, null, {}, .8], []]"
//		"{ \"h\\\"\\u00eq\\\\e\\/a\\\"a\" : 1.3e+1, \"obj\" :",
//		" {}, \"xi\" : {\"phi\" : \"omega\\u12\"}, \"\" : [",
//		"null, true, false], \"null\" : [true], \"doll",
//		"y\" : [], \"a\" : 0, \"b\" : 0., \"c\" : 0.0,",
//		" \"d\" : 1e-1, \"e\" : [\"done\"], \"f\" : \"",
//		"ok\", \"g\" : [{\"h\\u00e9\" : 2, \"i\" : null, \"j",
//		"\" : false, \"k\" : true}, null, {}, .8]}"
	};

	// The string stream to parse
	std::istringstream s;
	// Construct the parser, pass the stream to parse
	json::parser<char> parser(s);
	// In this case, the context used for callback invokations is a stack.
	// Note that it may anything, or even missing. It depends on how you
	// implement your callbacks.
	std::stack<json::internal_node *> ctx;
	// Set some callbacks. Note that this part is entirely optional.
	// Set the callback that is called when '{' is encountered.
	parser.hook_obj_start(reinterpret_cast<json::parser<char>::hook_start_end_t>(&obj_start_cb));
	// Set the callback that is called when a key in a key:value pair is encountered.
	parser.hook_key(reinterpret_cast<json::parser<char>::hook_key_t>(&key_cb));
	// Set the callback that is called when the constant value null or
	// a value of type string, number, or boolean is encountered in a key:value pair.
	parser.hook_obj_data(reinterpret_cast<json::parser<char>::hook_primitive_t>(&obj_data_cb));
	// Set the callback that is called when '}' is encountered. 
	parser.hook_obj_end(reinterpret_cast<json::parser<char>::hook_start_end_t>(&obj_end_cb));
	// Set the callback that is called when '[' is encountered.
	parser.hook_array_start(reinterpret_cast<json::parser<char>::hook_start_end_t>(&array_start_cb));
	// Set the callback that is called when the constant null or a value of type string, number,
	// or boolean is encountered in an array.
	parser.hook_array_data(reinterpret_cast<json::parser<char>::hook_primitive_t>(&array_data_cb));
	// Set the callback that is called when ']' is encountered.
	parser.hook_array_end(reinterpret_cast<json::parser<char>::hook_start_end_t>(&array_end_cb));
	// Set the context data that is passed to the callbacks.
	parser.set_context(&ctx);
	
	// The DOM tree root.
	json::root_node *root = new json::root_node();
	// Push it on the stack.
	ctx.push(root);
	
	json::parser<char>::result_t r;
	// parse the chunks.
	for (unsigned int i = 0; i < sizeof(chunks) / sizeof(chunks[0]); ++i) {
		// Set the internal buffer of the string stream to the chunk to be parsed.
		s.rdbuf()->pubsetbuf(const_cast<char *>(chunks[i]), strlen(chunks[i]));
		// Clear the flags of the input stream. This is necessary as each
		// invokation to the parser extracts characters from the stream (chunk)
		// until all are consumed or a scan/parse error is detected.
		// Thus, they always leave the EOF flag set unless an error is detected.
		s.clear();
		// Parse.
		r = parser.parse();
		// Parse cannot return OK in this phase because it does not know if
		// more chunks will arrive or not. The only values that can be returned
		// are ERROR and PENDING. Abort the parsing of the remaining chunks if ERROR.
		if (json::parser<char>::PENDING != r)
			break;
	}
	// Here we parsed all chunks. If no ERROR, then let the parser know that no more
	// chunks arrive. This is done by passing an empty chunk.
	// Sometimes it is not enough to pass an empty chunk just once.
	// Consider the following example:
	// The whole JSON is '{}', sent in one chunk.
	// The parser scans '{', then '}'. When it reads '}' is realises that '{}' is not
	// a valid token (scanning is greedy, so it tries to amass as many characters as possible
	// before returning a token). Hence, it returns the longest token encountered so far, i.e. '{'.
	// Upon the next invokation of the scanner, an attempt is made to fetch a new character
	// from the stream. However, end-of-stream (EOS) is reached. The scanner may not know if '}' is
	// a complete token or if new characters that can be amassed in the currently scanned token
	// will arrive in the new chunk. (Ok, we know that no token can start with '}' and contain other
	// characters, but the scanner does not have this look-ahead capacity. Imagin the casee of
	// numbers: we get 1. Should we decide that we have a token, namely the number one, or should
	// we wait for other characters, maybe another digit arrives that can be appended to 1.) So
	// the scanner does not return the '}' token, but returns PENDING. At this point we send an
	// empty chunk in order to signal that no more chunks arrive. The parser invokes the scanner.
	// The scanner encounters the EOS, so it knows that nothing will follow the '}'. Hence it will
	// return the token '}' to the parser. The parser is slightly puzzled: it has sent an empty
	// chunk from which a non-EOS token emerges. So we need to invoke the parser once more with
	// an empty chunk. This time, the scanner responds with EOS to the empty chunk and the parser
	// is relieved.
	while (json::parser<char>::PENDING == r) {
		s.rdbuf()->pubsetbuf(const_cast<char *>(""), 0);
		s.clear();
		r = parser.parse();
	}
	if (json::parser<char>::ERROR == r) {
		std::cerr << "Parse error" << std::endl;
		return 1;
	}
	// Pop the root of the DOM tree from the stack. If parsing is successful
	// the stack should contain only one element, the DOM tree root.
	json::root_node *n = reinterpret_cast<json::root_node *>(ctx.top());
	ctx.pop();
	// Print the DOM tree recursively.
	std::cout << *n << std::endl;
	// Delete it recursively.
	delete n;
	
	return 0;
}

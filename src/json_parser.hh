#ifndef __JSON_PARSER_HH__
#define __JSON_PARSER_HH__

#include <stack>
#include <stdexcept>
#include <string>
#include <istream>
#include "json_scanner.hh"

namespace json {

/**
 * \brief The json parser. Its public method is \link json::parser::parse parse\endlink.
 * It is an incremental parser adapted for streams, i.e. data may be incomplete
 * at the moment when parse is called. If new data arrives, \link json::parser::parse parse\endlink may be invoked again.
 * Check the unit tests for usage examples. 
 */
template<typename Char>
class parser {
public:
	/**
	 * \brief The results of \link json::parser::parse parse\endlink. PENDING indicates that parsing has not completed
	 * because there was insufficient data. It signals to the caller that more data
	 * is expected before the parser can decide if the input is legit.
	 */ 
	typedef enum {ERROR, PENDING, OK} result_t;
	/**
	 * \brief The constructor. It gets a _reference_ to an input stream. The input stream
	 * must be valid during the whole parsing process, i.e. it should not be destroyed
	 * between consecutive calls to \link json::parser::parse parse\endlink.
	 * 
	 * \param s A reference to the input stream containing the data to parse.
	 */
	parser(std::basic_istream<Char>&);
	/**
	 * \brief The parse method. It invokes the scanner that gets characters from the stream
	 * until either a scan or a parse error occurs or the end-of-stream is reached.
	 * 
	 * \return ERROR, PENDING, or OK.
	 * \exception std::runtime_error if an I/O error occurs when reading from the stream.
	 * std::logic_error for certain software bugs. 
	 */
	result_t parse();

	/**
	 * \brief Specifies the type of the callback that is invoked when a key in a
	 * key:value pair is encountered.
	 */
	typedef void (*hook_key_t)(const std::basic_string<Char>&, void *);
	/**
	 * \brief Specifies the type of the callback that is invoked when primitive
	 * data (strings, booleans, numbers, the null constant) are encountered as
	 * values in key:value pairs or in array elements.
	 */
	typedef void (*hook_primitive_t)(const std::basic_string<Char>&, int, void *);
	/**
	 * \brief Specifies the type of the callback that is invoked when objects or arrays start
	 * or end.
	 */
	typedef void (*hook_start_end_t)(void *);
	
	//! \brief Sets the callback that is called when '{' is encountered.
	inline void hook_obj_start(hook_start_end_t);
	//! \brief Sets the callback that is called when the key of a key:value pair is encountered.
	inline void hook_key(hook_key_t);
	//! \brief Sets the callback that is called when a value of primitive type is encountered in a key:value pair
	inline void hook_obj_data(hook_primitive_t);
	//! \brief Sets the callback that is called when '}' is encountered.
	inline void hook_obj_end(hook_start_end_t);
	//! \brief Sets the callback that is called when '[' is encountered.
	inline void hook_array_start(hook_start_end_t);
	//! \brief Sets the callback that is called when an array element of primitive type is encountered.
	inline void hook_array_data(hook_primitive_t);
	//! \brief Sets the callback that is called when ']' is encountered.
	inline void hook_array_end(hook_start_end_t);
	//! \brief Sets a context that is passed to every callback.
	inline void set_context(void *);
private:
	//! \brief The current state in the parser automaton.
	int crt;

	//! \brief The scanner.
	json::scanner<Char> scanner;

	//! \brief The stack that complements the parser automaton.
	std::stack<int> st;

	//! \brief The callback that is called when '{' is encountered.
	hook_start_end_t obj_start_cb;
	//! \brief The callback that is called when the key of a key:value pair is encountered.
	hook_key_t  key_cb;
	//! \brief The callback that is called when a value of primitive type is encountered in a key:value pair
	hook_primitive_t obj_data_cb;
	//! \brief The callback that is called when '}' is encountered.
	hook_start_end_t obj_end_cb;
	//! \brief The callback that is called when '[' is encountered.
	hook_start_end_t array_start_cb;
	//! \brief The callback that is called when an array element of primitive type is encountered.
	hook_primitive_t array_data_cb;
	//! \brief The callback that is called when ']' is encountered.
	hook_start_end_t array_end_cb;
	//! \brief The context that is passed to every callback.
	void *ctx;

	//! \brief The matrix elements type in the matrix representation of the automaton.
	typedef struct {
		/**
		 * \brief Specifies which action to take when a terminal or non-terminal is
		 * encountered.
		 * It is either SHIFT (i.e. -2), an ERROR (i.e. -1) or a positive integer.
		 * In the latter case, it indicates that the action to take is "reduce".
		 * The positive integer indicates which non-terminal to reduce. 
		 */
		int what;
		/**
		 * \brief If \link json::parser::pt_cell_t::what what\endlink is ERROR, then \link json::parser::pt_cell_t::where where\endlink is unused.
		 * If \link json::parser::pt_cell_t::what what\endlink is SHIFT, then \link json::parser::pt_cell_t::where where\endlink indicates the next state of the automaton.
		 * If \link json::parser::pt_cell_t::what what\endlink is REDUCE, then \link json::parser::pt_cell_t::where where\endlink indicates how many states to reduce from the stack. 
		 */
		unsigned int where;
	} pt_cell_t;

	//! \brief The matrix representation of the parse automaton.
//	static const pt_cell_t pt[37][19];
	static const pt_cell_t pt[38][18];

	typedef enum { SHIFT = -2} command_t;

	/**
	 * \brief Called for semantic actions. It invokes the callbacks that are set.
	 * 
	 * \param state The current state in the parse automaton
	 * \param token The currently scanned token.
	 * \param term The currently scanned terminal. This is needed in the case of primitive data
	 * in order to distinguish between, for example, the string token "1.2" and the number 1.2 or
	 * between the string token "false" and the boolean constant 'false'.
	 */	
	void semantics(int, const std::basic_string<Char>&, int);
};

//template<typename Char>
//const typename parser<Char>::pt_cell_t parser<Char>::pt[37][19] = {
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,1}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-2,12}, {-2,14}, {-1,0}, {-2,15}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,2}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,3}, {-1,0}, {-1,0}},
//{{-2,20}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,21}, {-2,19}, {-1,0}, {-1,0}, {-1,0}, {-2,10}, {-1,0}, {-2,6}, {-1,0}, {-1,0}, {-2,5}, {-1,0}, {-2,4}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {5,1}, {-1,0}, {-1,0}, {5,1}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {5,1}, {-1,0}, {-1,0}, {5,1}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-2,26}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,27}, {-2,25}, {-2,22}, {-2,24}, {-1,0}, {-2,11}, {-1,0}, {-2,9}, {7,0}, {-1,0}, {-2,8}, {-1,0}, {-2,7}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {5,1}, {5,1}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {5,1}, {5,1}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-2,26}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,27}, {-2,25}, {-2,35}, {-2,24}, {-1,0}, {-2,11}, {-1,0}, {-2,9}, {7,0}, {-1,0}, {-2,8}, {-1,0}, {-2,7}, {-1,0}},
//{{-1,0}, {-2,31}, {-2,14}, {-1,0}, {-2,15}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,2}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-2,33}, {-2,14}, {-1,0}, {-2,15}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,2}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,13}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {0,3}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {1,1}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-2,17}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {3,0}, {-1,0}, {-1,0}, {-2,16}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-2,18}, {-1,0}, {-2,15}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,2}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {2,2}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {3,2}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {5,1}, {-1,0}, {-1,0}, {5,1}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {5,1}, {-1,0}, {-1,0}, {5,1}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {4,3}, {-1,0}, {-1,0}, {4,3}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,23}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {6,3}, {-1,0}, {-1,0}, {6,3}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {7,1}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {5,1}, {5,1}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {5,1}, {5,1}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,28}, {-1,0}, {-1,0}, {-1,0}, {9,0}, {-2,29}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {8,2}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-2,26}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,27}, {-2,25}, {-1,0}, {-2,30}, {-1,0}, {-2,11}, {-1,0}, {-2,9}, {-1,0}, {-1,0}, {-2,8}, {-1,0}, {-2,7}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {9,2}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,32}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {0,3}, {-1,0}, {-1,0}, {0,3}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,34}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {0,3}, {0,3}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,36}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
//{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {6,3}, {6,3}, {-1,0}, {-1,0}, {-1,0}, {-1,0}}};

template<typename Char>
const typename parser<Char>::pt_cell_t parser<Char>::pt[38][18] = {
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,1}, {-1,0}, {-2,19}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-2,12}, {-2,14}, {-1,0}, {-2,15}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,2}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,3}, {-1,0}, {-1,0}},
{{-2,20}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,21}, {-1,0}, {-1,0}, {-1,0}, {-2,10}, {-1,0}, {-2,6}, {-1,0}, {-1,0}, {-2,5}, {-1,0}, {-2,4}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {5,1}, {-1,0}, {-1,0}, {5,1}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {5,1}, {-1,0}, {-1,0}, {5,1}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-2,26}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,27}, {-2,22}, {-2,24}, {-1,0}, {-2,11}, {-1,0}, {-2,9}, {6,0}, {-1,0}, {-2,8}, {-1,0}, {-2,7}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {5,1}, {5,1}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {5,1}, {5,1}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-2,26}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,27}, {-2,35}, {-2,24}, {-1,0}, {-2,11}, {-1,0}, {-2,9}, {6,0}, {-1,0}, {-2,8}, {-1,0}, {-2,7}, {-1,0}},
{{-1,0}, {-2,31}, {-2,14}, {-1,0}, {-2,15}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,2}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-2,33}, {-2,14}, {-1,0}, {-2,15}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,2}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,13}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {0,3}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {1,1}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-2,17}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {3,0}, {-1,0}, {-1,0}, {-2,16}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-2,18}, {-1,0}, {-2,15}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,2}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {2,2}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {3,2}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-2,26}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,27}, {-2,25}, {-2,24}, {-1,0}, {-2,11}, {-1,0}, {-2,9}, {6,0}, {-1,0}, {-2,8}, {-1,0}, {-2,7}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {5,1}, {-1,0}, {-1,0}, {5,1}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {4,3}, {-1,0}, {-1,0}, {4,3}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,23}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {0,3}, {-1,0}, {-1,0}, {0,3}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {6,1}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,37}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {5,1}, {5,1}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,28}, {-1,0}, {-1,0}, {-1,0}, {8,0}, {-2,29}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {7,2}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-2,26}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,27}, {-1,0}, {-2,30}, {-1,0}, {-2,11}, {-1,0}, {-2,9}, {-1,0}, {-1,0}, {-2,8}, {-1,0}, {-2,7}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {8,2}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,32}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {0,3}, {-1,0}, {-1,0}, {0,3}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,34}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {0,3}, {0,3}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-2,36}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {0,3}, {0,3}, {-1,0}, {-1,0}, {-1,0}, {-1,0}},
{{-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {-1,0}, {0,3}}};

template<typename Char>
parser<Char>::parser(std::basic_istream<Char>& s) :
	crt(0),
	scanner(s),
	obj_start_cb(0), key_cb(0), obj_data_cb(0), obj_end_cb(0),
	array_start_cb(0), array_data_cb(0), array_end_cb(0),
	ctx(0)
{
	s.unsetf(std::ios_base::skipws);
}

template<typename Char>
typename parser<Char>::result_t
parser<Char>::parse() {
	int term;
	std::basic_string<Char> token;

	term = scanner.get(token);
	if (term == json::scanner<Char>::ERROR)
		return ERROR;
	if (term == json::scanner<Char>::PENDING)
		return PENDING;

	do {
		switch (pt[crt][term].what) {
		case -1:
			return ERROR;
		case SHIFT:
			st.push(term);
			crt = pt[crt][term].where;
			st.push(crt);
			semantics(crt, token, term);
			term = scanner.get(token);
			if (term == json::scanner<Char>::ERROR)
				return ERROR;
			if (json::scanner<Char>::PENDING == term || json::scanner<Char>::EOS == term)
				return PENDING;
			break;
		default: // reduce
			int non_term = pt[crt][term].what;
			if (st.size() < pt[crt][term].where * 2)
				throw std::logic_error("Grammar error: Stack underflow.");
			for (unsigned int i = 0; i < pt[crt][term].where; ++i) {
				st.pop();
				st.pop();
			}
			if (st.empty()) {
				if (non_term != 0 || term != json::scanner<Char>::EOS)
					throw std::logic_error("Grammar error: Empty stack.");
				return OK;
			}
			crt = st.top();
			st.push(non_term);
			crt = pt[crt][non_term].where;
			if (-1 == crt)
				throw std::logic_error("Grammar error: Invalid arc.");
			st.push(crt);
			break;
		}
	} while (true);
}

template<typename Char>
void
parser<Char>::semantics(int state, const std::basic_string<Char>& token, int term) {
	switch (state) {
	// obj start
	case 1:
	case 10:
	case 11:
		if (0 != obj_start_cb)
			(*obj_start_cb)(ctx);
		break;
	// key
	case 2:
		if (0 != key_cb)
			(*key_cb)(token, ctx);
		break;
	// object end
	case 13:
	case 32:
	case 34:
		if (0 != obj_end_cb)
			(*obj_end_cb)(ctx);
		break;
	// object primitive data
	case 4:
	case 5:
		if (0 != obj_data_cb)
			(*obj_data_cb)(token, term, ctx);
		break;

	// array
	// array start
	case 6:
	case 9:
	case 19:
		if (0 != array_start_cb)
			(*array_start_cb)(ctx);
		break;
	// array end
	case 23:
	case 36:
	case 37:
		if (0 != array_end_cb)
			(*array_end_cb)(ctx);
		break;
	// array primitive data
	case 7:
	case 8:
		if (0 != array_data_cb)
			(*array_data_cb)(token, term, ctx);
		break;
	default:
		break;
	}
}

template<typename Char>
inline void
parser<Char>::hook_obj_start(hook_start_end_t cb) {
	obj_start_cb = cb;
}

template<typename Char>
inline void
parser<Char>::hook_key(hook_key_t cb) {
	key_cb = cb;
}

template<typename Char>
inline void
parser<Char>::hook_obj_data(hook_primitive_t cb) {
	obj_data_cb = cb;
}

template<typename Char>
inline void
parser<Char>::hook_obj_end(hook_start_end_t cb) {
	obj_end_cb = cb;
}

template<typename Char>
inline void
parser<Char>::hook_array_start(hook_start_end_t cb) {
	array_start_cb = cb;
}

template<typename Char>
inline void
parser<Char>::hook_array_data(hook_primitive_t cb) {
	array_data_cb = cb;
}

template<typename Char>
inline void
parser<Char>::hook_array_end(hook_start_end_t cb) {
	array_end_cb = cb;
}

template<typename Char>
inline void
parser<Char>::set_context(void *ctx_) {
	ctx = ctx_;
}

}
#endif

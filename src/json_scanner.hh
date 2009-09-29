#ifndef __JSON_SCANNER_HH__
#define __JSON_SCANNER_HH__

#include <istream>
#include <sstream>
#include <string>
#include <cctype>
#include <stdexcept>

namespace json {

/**
 * \brief The json scanner. The only public method is \link json::scanner::get get\endlink.
 * 
 * The scanning is incremental, i.e. if end-of-stream occurs in the middle of an token,
 * the scanner does not return a scan error but it returns PENDING, signalling to the caller
 * that input data is incompletely scanned.
 * After data is appended to the stream, the client may invoke the scanning method (\link json::scanner::get get\endlink) again.
 * The scanner resumes from where it left.
 * 
 * For example, the stream contains initially '{ "hell'. Invoking the scanner twice results
 * in getting L_BRACE PENDING. Next, 'o" : 12' is added to the stream. Subsequently invoking the
 * scanner three times results in getting STRING("hello") COLON PENDING. Next, '3.4} ' is added to the
 * stream. Subsequently invoking the scanner three times results in getting OTHER(123.4) R_BRACE PENDING.
 * In order to signal to the scanner that nothing will be added to the stream and that scanning must
 * complete, the scanner is invoked one more time, without adding anything to the stream. In this
 * case, the scanner returns EOS.
 * 
 * Consider the stream containing '{ "hello" : "doll'. After invoking the scanner four times we obtain
 * L_BRACE STRING("hello") COLON PENDING. Next, we signal to the scanner that nothing will be added to
 * the stream and that scanning must complete. Thus, we invoke the scanner one more time. This time
 * the scanner will return ERROR because the string "doll" is unfinished (no closing quote) and the
 * scanner is informed that nothing will be added to the stream anymore, i.e. "doll" without the closing
 * quote is really the last thing in the stream.
 * 
 * template<typename Char> std::basic_istream<Char>& operator>>(std::basic_istream<Char>&, Char&) or
 * template<typename Char> std::basic_istream<Char>& std::basic_istream<Char>::operator>>(Char&)
 * must be defined
 * 
 */
template<typename Char>
class scanner {
public:
	//! The tokens IDs that may be returned by the scanner
	typedef enum {ERROR = -1, L_BRACE = 9, R_BRACE = 10, L_BRACKET = 11, R_BRACKET = 12,
		COMMA = 13, STRING = 14, COLON = 15, OTHER = 16, EOS = 17, PENDING = 18} token_t;

	/**
	 * \brief The scanner constructor. Gets a _reference_ to an istream. Scanning is performed on this
	 * istream. Therefore, clients should not destroy the stream between calls to the parser.
	 * If no scan error occurs, the scanner reads from the stream until end-of-stream is reached and the
	 * eof flag is set. Therefore, the caller is responsible for clearing the state flags (eof flag)
	 * of the stream between consecutive calls.
	 * 
	 * \param b The istream to scan.
	 */  
	scanner(std::basic_istream<Char>&);
	/**
	 * \brief The scanning method. Scans the istream that was passed in the constructor.
	 * 
	 * \param token A string that is filled by the method with the scanned token. This is used for
	 * communicating the string to the caller and for determining the type of the OTHER tokens. For
	 * example, "123" (with quotes) will return STRING and the argument token will contain the string
	 * "123" while 123 (without quotes) will return OTHER and the argument token will contain the string
	 * "123". Similarly, "true", "false", and "null" (with quotes) will return STRING,
	 * while 'true', 'false', 'null' (without quotes) will return OTHER. In all cases, 'token' will contain
	 * the string "true", "false", or "null" respectively. 
	 * \return The token that was scanned. One of ERROR, L_BRACE, R_BRACE, L_BRACKET, R_BRACKET, COMMA,
	 * STRING, COLON, OTHER, EOS, and PENDING.
	 * \exception std::runtime_error if an I/O error occurs while reading from the istream. 
	 */
	token_t get(std::basic_string<Char>&);

private:
	/**
	 * \brief Characters have different meanings depending on the context. For example, '4' is a
	 * simple character if encountered between quotes and it is the number four otherwise. Or 'n' is
	 * the 14th letter of the alphabet, but it denotes line-feed if preceded by a backslash.
	 * This enumeration lists the possible contexts. 
	 */
	typedef enum { BACKSLASH_CONTEXT, STRING_CONTEXT, DFLT_CONTEXT} context_t;
	/**
	 * \brief The list of character categories that label arcs in the DFA.
	 * 
	 * The character categories are:
	 * - the letter 'A' (appears in 'false')
	 * - the letter 'E' (appears in 'false' and in 'true')
	 * - the letter 'F' (appears in 'false')
	 * - the letter 'L' (appears in 'false' and in 'null')
	 * - the letter 'N' (appears in 'null')
	 * - the letter 'R' (appears in 'true')
	 * - the letter 'S' (appears in 'false')
	 * - the letter 'T' (appears in 'true')
	 * - the letter 'U' (appears in 'true' and in 'null')
	 * - {}[],: (opening/closing brace/square bracket, comma, and colon)
	 * - the digits 1 to 9 inclusive.
	 * - . (dot)
	 * - +- (plus and minus)
	 * - \ (backslash)
	 * - " (quote)
	 * - any character except quote and backslash (the two characters with special meaning in the string context)
	 * - any character
	 * - whitespace: space, line feed, carriage return, page feed, tab
	 * - 0 (the number zero)
	 * .
	 */
	typedef enum { A = 0, E = 1, F = 2, L = 3, N = 4, R = 5, S = 6, T = 7,
		 U = 8, PUNCT = 9, DIGIT = 10, DOT = 11, SIGN = 12,
		 BACKSLASH = 13, QUOTE = 14, NOSPECIAL = 15, ANY = 16,
		 BLANK = 17, ZERO = 18} symbols_t;

	/**
	 * \brief Maps a character to a character category depending on the context.
	 * 
	 * \param c The character to map.
	 * \return The category the character belongs to.
	 */
	symbols_t translate(const Char&);
	/**
	 * \brief Given a character in the PUNCT character category, it returns the corresponding token.
	 * 
	 * \param c The character in the PUNCT character category.
	 * \return The corresponding token ( '{' -> L_BRACE, ':' -> COLON, etc)
	 */
	token_t punctuation(const Char&) const;
	/**
	 * \brief This function is invoked by \link json::scanner::get get\endlink when scanning was successful in finding a token.
	 * This function fills the argument with the substring of the input that constitutes the token
	 * and returns the integer value corresponding to the token. It invokes \link json::scanner::punctuation punctuation\endlink if the
	 * scanned character is in the PUNCT category. It buffers in \link json::scanner::buf buf\endlink the characters that it read from the
	 * stream but did not include in the token that is currently returned.
	 * 
	 * \param token The substring of the input stream that constitutes the token that is currently returned.
	 * \return The integer value corresponding to the token.
	 */
	token_t success(std::basic_string<Char>&);
	
	/**
	 * \brief Transforms a unicode character (16 bits) in a UTF-8 character
	 * (a sequence of 1 to 3 8-bit characters).
	 * 
	 * \param unicode The unicode character to transform
	 * \return The UTF-8 character
	 */
	std::basic_string<Char> unicode2utf8(const unsigned short) const;
	/**
	 * \brief This function is invoked on string tokens. It replaces the escape sequences in the string.
	 * 
	 * \param data The string to be postprocessed
	 * \param len The length of the string
	 * \return The postprocessed string
	 */
	const std::basic_string<Char> postprocess(const std::basic_string<Char>&, size_t) const;
	/**
	 * \brief Resets the scanner, i.e. sets the current state to the initial state of the DFA and clears the
	 * buffer in which the token that is currently scanned is buffered. However, it does not flush \link json::scanner::buf buf\endlink, the buffer
	 * in which data that was read from the stream but not used in any token is kept for being retrieved in
	 * a subsequent invokation of \link json::scanner::get get\endlink.
	 */
	inline void reset();
	/**
	 * \brief Upon successful detection of a token, the data that was read from the stream but not included
	 * in the found token is pushed in an internal buffer.
	 */
	inline void unget();
	/**
	 * \brief First it attempts to get a character from \link json::scanner::buf buf\endlink, the internal buffer in which characters read
	 * from the input stream but not used in the last returned token are stored. If \link json::scanner::buf buf\endlink is empty, it attempts
	 * to get a character from the input stream.
	 * 
	 * \param c The character that is got from \link json::scanner::buf buf\endlink or from the input stream
	 * \exception runtime_error if an I/O error occurs during reading from the input stream.
	 */
	void get(Char&);

	/**
	 * \brief A reference to the input stream.
	 */
	std::basic_istream<Char>& str;
	/**
	 * \brief The buffer in which the currently scanned token is buffered.
	 */
	std::basic_string<Char> data;
	/**
	 * \brief The buffer in which characters that were read from the input stream but not used in the last
	 * returned token are stored for a subsequent invokation of the scanner.
	 */
	std::basic_stringstream<Char> buf;

	/**
	 * \brief The current DFA state.
	 */
	int crt;
	/**
	 * \brief The last accepting DFA state that was encountered. -1 If no accepting DFA state has been
	 * encountered yet.
	 */
	int last_final;
	/**
	 * \brief The context. \sa #context_t.
	 */
	context_t context;
	/**
	 * \brief Indicates how many characters have been read from the input stream but not yet included in a token.
	 * They are stored in \link json::scanner::data data\endlink. After a token is detected, the part of \link json::scanner::data data\endlink that forms the token is returned to
	 * the caller and the length of the token is subtracted from to_unget. The remaining characters (the ones read
	 * but not included in the token) are then stored ("ungot") in \link json::scanner::buf buf\endlink in order to be retrieved
	 * in a subsequent call to \link json::scanner::get get\endlink.
	 */
	unsigned int to_unget;

	/**
	 * \brief The DFA.
	 */
	static const int st[28][19];
	/**
	 * \brief An array indicating if the DFA state corresponding to the array entry is an accepting state.
	 * If yes, the array indicates which token the state accepts.
	 */
	static const int final[28];
};

template<typename Char> const int
scanner<Char>::st[28][19] =
	// A   E   F   L   N   R   S   T   U {}[]:, 1-9 \. +-  \   " [^"\] . ' \t\r\n\f'
	{{-1, -1, 16, -1,  7, -1, -1, 11, -1, 15,  2, 22, 27, -1,  1, -1, -1,  0, 21},
	 {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  5,  4,  3, -1, -1, -1},
	 {-1, 24, -1, -1, -1, -1, -1, -1, -1, -1,  2, 23, -1, -1,  1, -1, -1, -1,  2},
	 {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  5,  4,  3, -1, -1, -1},
	 {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	 {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  6, -1, -1},
	 {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  5,  4,  3, -1, -1, -1},
	 {-1, -1, -1, -1, -1, -1, -1, -1,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	 {-1, -1, -1,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	 {-1, -1, -1, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	 {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	 {-1, -1, -1, -1, -1, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	 {-1, -1, -1, -1, -1, -1, -1, -1, 13, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	 {-1, 14, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	 {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	 {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	 {17, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	 {-1, -1, -1, 18, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	 {-1, -1, -1, -1, -1, -1, 19, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	 {-1, 20, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	 {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	 {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 23, -1, -1, -1, -1, -1, -1. -1},
	 {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 23, -1, -1, -1, -1, -1, -1, -1, 23},
	 {-1, 24, -1, -1, -1, -1, -1, -1, -1, -1, 23, -1, -1, -1, -1, -1, -1, -1, 23},
	 {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 26, -1, 25, -1, -1, -1, -1, -1, -1},
	 {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 26, -1, -1, -1, -1, -1, -1, -1, -1},
	 {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 26, -1, -1, -1, -1, -1, -1, -1, 26},
	 {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  2, 22, -1, -1, -1, -1, -1, -1, 21}};

template<typename Char>
const int
scanner<Char>::final[] = {0, 0, OTHER, 0, STRING, 0, 0, 0, 0, 0, OTHER, 0, 0, 0, OTHER, PUNCT, 0, 0, 0, 0, OTHER, OTHER, 0, OTHER, 0, 0, OTHER, 0};

template<typename Char>
scanner<Char>::scanner(std::basic_istream<Char>& b) :
	str(b),
	crt(0),
	last_final(-1),
	context(DFLT_CONTEXT),
	to_unget(0)
{
}

template<typename Char>
inline void 
scanner<Char>::reset() {
	crt = 0;
	context = DFLT_CONTEXT;
	data.clear();
}

// called from success.
template<typename Char>
inline void
scanner<Char>::unget() {
	std::basic_string<Char> internal(buf.str());
	// discard the characters already read.
	internal.erase(0, buf.tellg());
	// push the characters in data that were not used in the last detected token
	internal.insert(0, data.substr(data.size() - to_unget));
	buf.str(internal);
	// clear the eof flag
	buf.clear();
	to_unget = 0;
}

template<typename Char>
typename scanner<Char>::symbols_t
scanner<Char>::translate(const Char& c) {
	switch (context) {

	case STRING_CONTEXT:
		switch (c) {
		case static_cast<Char>('\\'):
			context = BACKSLASH_CONTEXT;
			return BACKSLASH;
		case static_cast<Char>('\"'):
			context = DFLT_CONTEXT;
			return QUOTE;
		default:
			return NOSPECIAL;
		}

	case BACKSLASH_CONTEXT:
		context = STRING_CONTEXT;
		return ANY;

	default:
		switch (c) {
		case static_cast<Char>('\"'):
			context = STRING_CONTEXT;
			return QUOTE;
		case static_cast<Char>('0'):
			return ZERO;
		case static_cast<Char>('1'):
		case static_cast<Char>('2'):
		case static_cast<Char>('3'):
		case static_cast<Char>('4'):
		case static_cast<Char>('5'):
		case static_cast<Char>('6'):
		case static_cast<Char>('7'):
		case static_cast<Char>('8'):
		case static_cast<Char>('9'):
			return DIGIT;
		case static_cast<Char>('.'):
			return DOT;
		case static_cast<Char>('E'):
		case static_cast<Char>('e'):
			return E;
		case static_cast<Char>('+'):
		case static_cast<Char>('-'):
			return SIGN;
		case static_cast<Char>('{'):
		case static_cast<Char>('}'):
		case static_cast<Char>('['):
		case static_cast<Char>(']'):
		case static_cast<Char>(','):
		case static_cast<Char>(':'):
			return PUNCT;
		case static_cast<Char>('\n'):
		case static_cast<Char>('\r'):
		case static_cast<Char>(' '):
		case static_cast<Char>('\t'):
		case static_cast<Char>('\f'):
			return BLANK;
		case static_cast<Char>('A'):
		case static_cast<Char>('a'):
			return A;
		case static_cast<Char>('F'):
		case static_cast<Char>('f'):
			return F;
		case static_cast<Char>('L'):
		case static_cast<Char>('l'):
			return L;
		case static_cast<Char>('N'):
		case static_cast<Char>('n'):
			return N;
		case static_cast<Char>('R'):
		case static_cast<Char>('r'):
			return R;
		case static_cast<Char>('S'):
		case static_cast<Char>('s'):
			return S;
		case static_cast<Char>('T'):
		case static_cast<Char>('t'):
			return T;
		case static_cast<Char>('U'):
		case static_cast<Char>('u'):
			return U;
		default:
			return ANY;
		}
	}
}

template<typename Char>
typename scanner<Char>::token_t
scanner<Char>::punctuation(const Char& c) const {
	switch (c) {
	case static_cast<Char>('{'):
		return L_BRACE;
	case static_cast<Char>('}'):
		return R_BRACE;
	case static_cast<Char>('['):
		return L_BRACKET;
	case static_cast<Char>(']'):
		return R_BRACKET;
	case static_cast<Char>(':'):
		return COLON;
	case static_cast<Char>(','):
		return COMMA;
	default:
		throw std::logic_error("Punctuation");
	}
}

template<typename Char>
const std::basic_string<Char>
scanner<Char>::postprocess(const std::basic_string<Char>& data, size_t len) const {
	std::basic_string<Char> r;
	for (size_t i = 1; i < len; ++i) {
		Char c = data[i];
		if (c != static_cast<Char>('\\'))
			r.push_back(static_cast<Char>(c));
		else {
			++i;
			c = data[i];
			switch (c) {
			case static_cast<Char>('\\'):
			case static_cast<Char>('/'):
			case static_cast<Char>('"'):
				r.push_back(static_cast<Char>(c));
				break;
			case static_cast<Char>('t'):
				r.push_back(static_cast<Char>('\t'));
				break;
			case static_cast<Char>('n'):
				r.push_back(static_cast<Char>('\n'));
				break;
			case static_cast<Char>('r'):
				r.push_back(static_cast<Char>('\r'));
				break;
			case static_cast<Char>('f'):
				r.push_back(static_cast<Char>('\f'));
				break;
			case static_cast<Char>('b'):
				r.push_back(static_cast<Char>('\b'));
				break;
			case static_cast<Char>('u'):
				if (i + 4 < len) {
					std::basic_istringstream<Char> s(data.substr(i + 1, 4));
					unsigned short unicode;
					s >> std::hex >> unicode;
					if (s.eof()) {
						r.append(unicode2utf8(unicode)); 
						i += 4;
						break;
					}
				}
				r.push_back(static_cast<Char>('u'));
				break;
			default:
				r.push_back(c);
				break;
			}
		}
	}
	return r;
}

template<typename Char>
typename scanner<Char>::token_t
scanner<Char>::success(std::basic_string<Char>& token) {
	int terminal = final[last_final];
	last_final = -1;
	if (terminal == STRING)
		token = postprocess(data, data.size() - to_unget - 1);
	else
		token = data.substr(0, data.size() - to_unget);
	if (to_unget > 0)
		unget();
	reset();
	if (terminal != PUNCT)
		return static_cast<token_t>(terminal);
	return punctuation(token.at(0));
}

template<typename Char>
void
scanner<Char>::get(Char& c) {
	buf >> c;
	if (buf.bad())
		throw std::runtime_error("I/O");
	if (buf.eof()) {
		str >> c;
		if (str.bad())
			throw std::runtime_error("I/O");
	}
}

template<typename Char>
typename scanner<Char>::token_t
scanner<Char>::get(std::basic_string<Char>& token) {
	Char c;
	get(c);
	if (buf.eof() && str.eof()) {
		if (last_final != -1)
			return success(token);
		if (to_unget > 0) {
			reset();
			return ERROR;
		}
		return EOS;
	}
	symbols_t type = translate(c);
	if (type != BLANK) {
		data.push_back(c);
		++to_unget;
	}
	do {
		crt = st[crt][type];
		if (-1 == crt) {
			if (last_final != -1)
				return success(token);
			else {
				reset();
				return ERROR;
			}
		}
		if (final[crt] != 0) {
			last_final = crt;
			to_unget = 0;
		}
		get(c);
		if (buf.eof() && str.eof())
			return PENDING;
		type = translate(c);
		if (type != BLANK) {
			data.push_back(c);
			++to_unget;
		}
	} while (true);
}

}

#endif

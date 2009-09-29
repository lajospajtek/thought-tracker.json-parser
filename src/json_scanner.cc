#include <string>
#include "json_scanner.hh"

namespace json {

/**
 * \brief Transforms a unicode character (16 bits) in a UTF-8 character (a sequence of 1 to 3 8-bit characters).
 * 
 * \param unicode The unicode character to transform
 * \return The UTF-8 character
 */
template<>
std::basic_string<char>
scanner<char>::unicode2utf8(const unsigned short unicode) const {
	unsigned int utf8 = 0;
	char *p = reinterpret_cast<char *>(&utf8);

	if (unicode < 0x0080)
		p[0] = static_cast<char>(unicode & 0x007f);
	else if (unicode < 0x0800) {
		p[0] = static_cast<char>(0x00c0 | (unicode >> 6));
		p[1] = static_cast<char>(0x0080 | (unicode & 0x003f));
	} else {
		p[0] = static_cast<char>(0x00e0 | (unicode >> 12));
		p[1] = static_cast<char>(0x0080 | ((unicode >> 6) & 0x03ff));
		p[2] = static_cast<char>(0x0080 | (unicode & 0x003f));
	}
	return std::basic_string<char>(p);
}

}

#ifndef __Text_h__
#define __Text_h__

#include "os/Exception.h"
#include <codecvt>
#include <locale>
#include <string>

/**
        @todo Test lowercase on 3 and 4 byte utf8 sequences
        @todo Test to/from Hex
*/
namespace text {

/// @brief Append to output parameter or clear it first
enum ClearFirst {
  AppendToOutput,  ///< The parameter being used for output will be appended to
  ClearOutputFirst ///< The output parameter will be cleared before appending
};

/** Convert a utf8 string to a wide string.
        @param utf8 a UTF-8 encoded string
        @param wide The wide string to receive the converted text
        @param clear Should we append to wide or clear it first, defaults to
   clearing first
        @return wide parameter
*/
inline std::wstring &convert(const std::string &utf8, std::wstring &wide,
                             ClearFirst clear = ClearOutputFirst) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wideconv;

  if (ClearOutputFirst == clear) {
    wide.clear(); // not tested
  }
  wide = wideconv.from_bytes(utf8);
  return wide;
}

/** Convert a wide string to a utf8 string.
        @param wide The wide string to convert
        @param utf8 receives the UTF-8 encoded string
        @param clear Should we append to utf8 or clear it first, defaults to
   clearing first
        @return utf8 parameter
*/
inline std::string &convert(const std::wstring &wide, std::string &utf8,
                            ClearFirst clear = ClearOutputFirst) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wideconv;

  if (ClearOutputFirst == clear) {
    utf8.clear(); // tested in libernet tests
  }
  utf8 = wideconv.to_bytes(wide);
  return utf8;
}

/** Convert a wide string to lowercase.
        @param mixed the mixed-case string
        @param lower Will contain mixed, but all characters are lower case
        @param clear Should lower be cleared first or appended with the lower
   case string. defaults to clear.
        @return lower parameter
*/
inline std::wstring &tolower(const std::wstring &mixed, std::wstring &lower,
                             ClearFirst clear = ClearOutputFirst) {
  std::locale utf8Locale("en_US.UTF-8");

  if (ClearOutputFirst == clear) {
    lower.clear(); // not tested
  }
  lower.reserve(lower.size() + mixed.size());
  for (auto c : mixed) {
    lower.append(1, std::tolower(c, utf8Locale));
  }
  return lower;
}

/** Convert a utf8 string to lowercase.
        @param mixedUTF8 the mixed-case string
        @param lower Will contain mixed, but all characters are lower case
        @param clear Should lower be cleared first or appended with the lower
   case string. defaults to clear.
        @return lower parameter
*/
inline std::string &tolower(const std::string &mixedUTF8, std::string &lower,
                            ClearFirst clear = ClearOutputFirst) {
  std::wstring wmixed, wlower;

  return convert(tolower(convert(mixedUTF8, wmixed, AppendToOutput), wlower,
                         AppendToOutput),
                 lower, clear);
}

/** Convert a utf8 string to lowercase.
        @param mixedUTF8 the mixed-case string
        @return lowercase version of mixedUTF8
*/
inline std::string tolower(const std::string &mixedUTF8) {
  std::string buffer;

  return tolower(mixedUTF8, buffer, AppendToOutput);
}

/** Get the hexadecimal representation of a binary string.
        @param binary Buffer of binary data
        @param hex receives the hex representation of binary
        @param clear Should hex be cleared first or appended with the hex.
   defaults to clear.
        @return hex parameter
*/
inline std::string &toHex(const std::string &binary, std::string &hex,
                          ClearFirst clear = ClearOutputFirst) {
  const char *const hexDigits = "0123456789abcdef";

  if (ClearOutputFirst == clear) {
    hex.clear();
  }
  for (int i = 0; (i < static_cast<int>(binary.size())); ++i) {
    const int lowerIndex = (binary[i] >> 4) & 0x0F;
    const int upperIndex = binary[i] & 0x0F;

    hex.append(1, hexDigits[lowerIndex]);
    hex.append(1, hexDigits[upperIndex]);
  }
  return hex;
}

/** Get the hexadecimal representation of a binary string.
        @param binary Buffer of binary data
        @return hex representation of binary
*/
inline std::string toHex(const std::string &binary) {
  std::string buffer;

  return toHex(binary, buffer, AppendToOutput);
}

/** Get the binary represented by a hexadecimal string.
        @param hex the hex representation of a binary string
        @param binary receives the binary represented by hex
        @param clear Should binary be cleared first or appended with the hex.
   defaults to clear.
        @return binary parameter
*/
inline std::string &fromHex(const std::string &hex, std::string &binary,
                            ClearFirst clear = ClearOutputFirst) {
  std::string hexDigits("0123456789abcdef");

  if (ClearOutputFirst == clear) {
    binary.clear();
  }
  AssertMessageException(hex.size() % 2 == 0);
  for (int byte = 0; byte < static_cast<int>(hex.size() / 2); ++byte) {
    const int nibble1 = byte * 2 + 1;
    std::string::size_type found1 = hexDigits.find(hex[nibble1]);

    const int nibble2 = nibble1 - 1;
    std::string::size_type found2 = hexDigits.find(hex[nibble2]);

    AssertMessageException(found1 != std::string::npos);
    AssertMessageException(found2 != std::string::npos);
    binary.append(1, (found2 << 4) | found1);
  }
  return binary;
}

/** Get the binary represented by a hexadecimal string.
        @param hex the hex representation of a binary string
        @return binary represented by hexs
*/
inline std::string fromHex(const std::string &hex) {
  std::string buffer;

  return fromHex(hex, buffer, AppendToOutput);
}

/// @brief The base characters used in all base64 encodings
#define __base_64_base_characters                                              \
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
#define __base_64_standard_extension "+/" ///< std variation of base64 chars
#define __base_64_url_extension "-_"      ///< URL variation of base64 chars
#define __base_64_standard_trailer "="    ///< std base64 trailing char
#define __base_64_url_trailer "." ///< URL variation of base64 trailing char

/// @brief Which style of bas64 to generate
enum Base64Style {
  Base64,            ///< Standard base64 encoding
  Base64URL,         ///< URL-safe base64 encoding
  Base64NoPadding,   ///< Standard base64 encoding but no zero padding
  Base64URLNoPadding ///< URL-safe base64 encoding but no zero padding

};

/// @brief Common line-size boundaries
enum CommonSplits {
  DoNotSplitBase64 = 0,   ///< Do not split into lines
  SplitBase64ForPEM = 64, ///< Line wrap for PEM wrapper
  SplitBase64ForMIME = 76 ///< Line wrap for MIME wrapper
};

/** Convert a binary string into base 64 encoding.
        @param binary a buffer of binary data
        @param base64 string to receive the base64 encoding
        @param style standard or url-safe base64 encoding
        @param split how many characters per line. 0 means no line splits.
   defaults to 0.
        @param eol The characters to use for end of line. defaults to \\n
        @param clear Should base64 be cleared first or appended with the hex.
   defaults to clear.
        @return base64 parameter
*/
inline std::string &base64Encode(const std::string &binary, std::string &base64,
                                 Base64Style style = Base64,
                                 int split = DoNotSplitBase64,
                                 const std::string &eol = "\n",
                                 ClearFirst clear = ClearOutputFirst) {
  const bool urlStyle = (Base64URL == style) || (Base64URLNoPadding == style);
  const bool addPadding = (Base64 == style) || (Base64URL == style);
  const std::string characters(
      urlStyle ? __base_64_base_characters __base_64_url_extension
               : __base_64_base_characters __base_64_standard_extension);
  const char padding =
      (urlStyle ? __base_64_url_trailer : __base_64_standard_trailer)[0];
  const int size = binary.size();

  if (ClearOutputFirst == clear) {
    base64.clear();
  }

  base64.reserve(base64.size() + (size + 2) / 3 * 4);

  for (int i = 0; i < size; i += 3) {
    const auto twoOrMoreBytes = i + 1 < size;
    const auto threeBytes = i + 2 < size;
    const unsigned char b1 = binary[i];
    const unsigned char b2 = twoOrMoreBytes ? binary[i + 1] : 0;
    const unsigned char b3 = threeBytes ? binary[i + 2] : 0;
    const unsigned char c1 = ((b1 & 0xfc) >> 2);
    const unsigned char c1_2 = ((b1 & 0x03) << 4);
    const unsigned char c2 = ((b1 & 0x03) << 4) | ((b2 & 0xf0) >> 4);
    const unsigned char c2_2 = ((b2 & 0x0f) << 2);
    const unsigned char c3 = ((b2 & 0x0f) << 2) | ((b3 & 0xc0) >> 6);
    const unsigned char c4 = (b3 & 0x3f);

    base64.append(1, characters[c1]);

    if (twoOrMoreBytes) {
      base64.append(1, characters[c2]);

      if (threeBytes) {
        base64.append(1, characters[c3]);
        base64.append(1, characters[c4]);
      } else {
        base64.append(1, characters[c2_2]);
        if (addPadding) {
          base64.append(1, padding);
        }
      }

    } else {
      base64.append(1, characters[c1_2]);
      if (addPadding) {
        base64.append(1, padding);
        base64.append(1, padding);
      }
    }
  }

  if ((split > 0) && (eol.size() > 0)) {
    std::string::size_type offset = split;

    while (offset < base64.size()) {
      base64.insert(offset, eol);
      offset += split + eol.size();
    }
  }

  return base64;
}

/** Convert a binary string into base 64 encoding.
        @param binary a buffer of binary data
        @param style standard or url-safe base64 encoding
        @param split how many characters per line. 0 means no line splits.
   defaults to 0.
        @param eol The characters to use for end of line. defaults to \\n
        @return base64 encoding of binary parameter
*/
inline std::string base64Encode(const std::string &binary,
                                Base64Style style = Base64,
                                int split = DoNotSplitBase64,
                                const std::string &eol = "\n") {
  std::string buffer;

  return base64Encode(binary, buffer, style, split, eol, AppendToOutput);
}

/** Determine the value of a base64 character.
        Will return the position of c in standardCharacters or urlCharacters.
        If c is not in either, it will check if it is in allTrailingCharacters,
   and return 0. If c is not in any of the three, it will throw an exception.
        @param c The base64 character to find the value of
        @param standardCharacters The standard base64 characters, in order
        @param urlCharacters The url variant base64 characters, in order
        @param allTrailingCharacters the set of both standard and url base64
   trailing characters
        @throws msg::Exception if the character is not a base64 character
*/
inline unsigned char
_base64CharacterValue(char c, const std::string &standardCharacters,
                      const std::string &urlCharacters,
                      const std::string &allTrailingCharacters) {
  const auto f1 = standardCharacters.find(c);

  if (f1 == std::string::npos) {
    const auto f2 = urlCharacters.find(c);

    if (f2 == std::string::npos) {
      AssertMessageException(allTrailingCharacters.find(c) != // not tested
                             std::string::npos);

      return 0; // not tested
    }
    return f2;
  }

  return f1;
}

/** Find the next whitespace character.
        @param s the string to search.
        @param offset the offset in s to start searching for whitespace.
   defaults to 0
        @return the offset of the next whitespace character after offset or
   std::string::npos if no whitespace is found.
*/
inline std::string::size_type
_findWhitespace(const std::string &s, std::string::size_type offset = 0) {
  while ((offset < s.size()) && !std::isspace(s[offset])) {
    offset += 1;
  }
  return offset < s.size() ? offset : std::string::npos;
}

/** Strips all whitespace characters from a string.
        Note: The original string (s) is not modified
        @param s The string to strip
        @param buffer A string that may contain s with whitespaces stripped
        @return If s has no whitespace, s is returned, otherwise, buffer is
   returned.
*/
inline const std::string &_stripWhitespace(const std::string &s,
                                           std::string &buffer) {
  std::string::size_type offset = _findWhitespace(s);

  if (std::string::npos == offset) {
    return s;
  }

  buffer = s;

  do {
    buffer.erase(offset, 1);
  } while (std::string::npos != (offset = _findWhitespace(buffer, offset)));

  return buffer;
}

/** Decode a base64 encoded string into a binary string.
        base64 can be standard or url encoded, or a mix. It can have any line
   ending or line length. It can have any whitespace dispersed within. It may or
   may not have the trailing padding characters, or even incomplete trailing
        characters.
        @param base64 base64 encoded string
        @param binary receives the binary version of base64
        @param clear Should base64 be cleared first or appended with the hex.
   defaults to clear.
        @return binary parameter
*/
inline std::string &base64Decode(const std::string &base64, std::string &binary,
                                 ClearFirst clear = ClearOutputFirst) {
  std::string buffer;
  const auto &source = _stripWhitespace(base64, buffer);
  const int size = source.size();
  const std::string characters(
      __base_64_base_characters __base_64_standard_extension);
  const std::string urlCharacters(
      __base_64_base_characters __base_64_url_extension);
  const std::string padding(__base_64_standard_trailer __base_64_url_trailer);

  if (ClearOutputFirst == clear) {
    binary.clear();
  }

  binary.reserve(size / 4 * 3);

  for (auto i = 0; i < size; i += 4) {
    const auto has2 = i + 1 < size;
    const auto has3 = i + 2 < size;
    const auto has4 = i + 3 < size;
    const auto c1 =
        _base64CharacterValue(source[i], characters, urlCharacters, padding);
    const auto c2 = has2 ? _base64CharacterValue(source[i + 1], characters,
                                                 urlCharacters, padding)
                         : 0;
    const auto c3p =
        has3 ? std::string::npos != padding.find(source[i + 2]) : true;
    const auto c3 = c3p ? 0
                        : _base64CharacterValue(source[i + 2], characters,
                                                urlCharacters, padding);
    const auto c4p =
        has4 ? std::string::npos != padding.find(source[i + 3]) : true;
    const auto c4 = c4p ? 0
                        : _base64CharacterValue(source[i + 3], characters,
                                                urlCharacters, padding);

    binary.append(1, (c1 << 2) | ((c2 & 0x30) >> 4));

    if (!c3p) {
      binary.append(1, ((c2 & 0x0f) << 4) | ((c3 & 0x3c) >> 2));

      if (!c4p) {
        binary.append(1, ((c3 & 0x03) << 6) | c4);
      }
    }
  }
  return binary;
}

/** Decode a base64 encoded string into a binary string.
        base64 can be standard or url encoded, or a mix. It can have any line
   ending or line length. It can have any whitespace dispersed within. It may or
   may not have the trailing padding characters, or even incomplete trailing
        characters.
        @param base64 base64 encoded string
        @return binary decoded from base64
*/
inline std::string base64Decode(const std::string &base64) {
  std::string buffer;

  return base64Decode(base64, buffer, AppendToOutput);
}

/** Counts the number of characters that match from the beginning of two
   strings.
        @param s1 first string to compare
        @param s2 second string to compare
        @return The size of the matching prefix of s1 and s2.
*/
inline int matching(const std::string &s1, const std::string &s2) {
  const int shorterLength = int(std::min(s1.size(), s2.size()));

  for (int i = 0; i < shorterLength; ++i) {
    if (s1[i] != s2[i]) {
      return i;
    }
  }
  return int(shorterLength);
}

/** Remove a character from the beginning and ending of a string.
        @param s The string to modify
        @param c The character to remove from the ends of the string. Defaults
   to space.
        @return s parameter
*/
inline std::string &trim(std::string &s, char c = ' ') {
  while ((s.size() > 0) && (s[0] == c)) {
    s.erase(0, 1);
  }
  while ((s.size() > 0) && (s[s.size() - 1] == c)) {
    s.erase(s.size() - 1);
  }
  return s;
}

// Don't pollute the preprocessor namespaces
#undef __base_64_base_characters
#undef __base_64_standard_extension
#undef __base_64_url_extension
#undef __base_64_standard_trailer
#undef __base_64_url_trailer

} // namespace text

#endif // __Text_h__

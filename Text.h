#ifndef __Text_h__
#define __Text_h__

#include "os/Exception.h"
#include <codecvt>
#include <locale>
#include <string>

/**
        @todo document
        @todo Test lowercase on 3 and 4 byte utf8 sequences
        @todo Test to/from Hex
*/
namespace text {

enum ClearFirst { AppendToOutput, ClearOutputFirst };

inline std::wstring &convert(const std::string &utf8, std::wstring &wide,
                             ClearFirst clear = ClearOutputFirst) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wideconv;

  if (ClearOutputFirst == clear) {
    wide.clear();
  }
  wide = wideconv.from_bytes(utf8);
  return wide;
}

inline std::string &convert(const std::wstring &wide, std::string &utf8,
                            ClearFirst clear = ClearOutputFirst) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wideconv;

  if (ClearOutputFirst == clear) {
    utf8.clear();
  }
  utf8 = wideconv.to_bytes(wide);
  return utf8;
}

inline std::wstring &tolower(const std::wstring &mixed, std::wstring &lower,
                             ClearFirst clear = ClearOutputFirst) {
  std::locale utf8Locale("en_US.UTF-8");

  if (ClearOutputFirst == clear) {
    lower.clear();
  }
  lower.reserve(lower.size() + mixed.size());
  for (auto c : mixed) {
    lower.append(1, std::tolower(c, utf8Locale));
  }
  return lower;
}

inline std::string &tolower(const std::string &mixed, std::string &lower,
                            ClearFirst clear = ClearOutputFirst) {
  std::wstring wmixed, wlower;

  return convert(
      tolower(convert(mixed, wmixed, AppendToOutput), wlower, AppendToOutput),
      lower, clear);
}

inline std::string tolower(const std::string &mixed) {
  std::string buffer;

  return tolower(mixed, buffer, AppendToOutput);
}

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

inline std::string toHex(const std::string &binary) {
  std::string buffer;

  return toHex(binary, buffer, AppendToOutput);
}

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

inline std::string fromHex(const std::string &hex) {
  std::string buffer;

  return fromHex(hex, buffer, AppendToOutput);
}

#define __base_64_base_characters                                              \
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
#define __base_64_standard_extension "+/"
#define __base_64_url_extension "-_"
#define __base_64_standard_trailer "="
#define __base_64_url_trailer "."

enum Base64Style {
  Base64,
  Base64URL

};
enum CommonSplits {
  DoNotSplitBase64 = 0,
  SplitBase64ForPEM = 64,
  SplitBase64ForMIME = 76
};

inline std::string &base64Encode(const std::string &binary, Base64Style style,
                                 std::string &base64,
                                 int split = DoNotSplitBase64,
                                 const std::string &eol = "\n",
                                 ClearFirst clear = ClearOutputFirst) {
  const bool urlStyle = Base64URL == style;
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
        base64.append(1, padding);
      }

    } else {
      base64.append(1, characters[c1_2]);
      base64.append(1, padding);
      base64.append(1, padding);
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

inline std::string base64Encode(const std::string &binary,
                                Base64Style style = Base64,
                                int split = DoNotSplitBase64,
                                const std::string &eol = "\n") {
  std::string buffer;

  return base64Encode(binary, style, buffer, split, eol, AppendToOutput);
}

inline unsigned char _find(char c, const std::string &s1, const std::string &s2,
                           const std::string &p) {
  const auto f1 = s1.find(c);

  if (f1 == std::string::npos) {
    const auto f2 = s2.find(c);

    if (f2 == std::string::npos) {
      AssertMessageException(p.find(c) != std::string::npos);

      return 0;
    }
    return f2;
  }

  return f1;
}

inline std::string::size_type
_findWhitespace(const std::string &s, std::string::size_type offset = 0) {
  while ((offset < s.size()) && !std::isspace(s[offset])) {
    offset += 1;
  }
  return offset < s.size() ? offset : std::string::npos;
}

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
    const auto c1 = _find(source[i], characters, urlCharacters, padding);
    const auto c2 =
        has2 ? _find(source[i + 1], characters, urlCharacters, padding) : 0;
    const auto c3p =
        has3 ? std::string::npos != padding.find(source[i + 2]) : true;
    const auto c3 =
        c3p ? 0 : _find(source[i + 2], characters, urlCharacters, padding);
    const auto c4p =
        has4 ? std::string::npos != padding.find(source[i + 3]) : true;
    const auto c4 =
        c4p ? 0 : _find(source[i + 3], characters, urlCharacters, padding);

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

inline std::string base64Decode(const std::string &base64) {
  std::string buffer;

  return base64Decode(base64, buffer, AppendToOutput);
}

inline int matching(const std::string &s1, const std::string &s2) {
  const int shorterLength = int(std::min(s1.size(), s2.size()));

  for (int i = 0; i < shorterLength; ++i) {
    if (s1[i] != s2[i]) {
      return i;
    }
  }
  return int(shorterLength);
}

inline std::string &trim(std::string &s, char c=' ') {
	while( (s.size() > 0) && (s[0] == c) ) {
		s.erase(0, 1);
	}
	while( (s.size() > 0) && (s[s.size() - 1] == c) ) {
		s.erase(s.size() - 1);
	}
	return s;
}

#undef __base_64_base_characters
#undef __base_64_standard_extension
#undef __base_64_url_extension
#undef __base_64_standard_trailer
#undef __base_64_url_trailer

} // namespace text

#endif // __Text_h__

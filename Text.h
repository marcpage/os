#ifndef __Text_h__
#define __Text_h__

#include "os/Exception.h"
#include <codecvt>
#include <locale>
#include <string>

/**
        @todo document
        @todo Test lowercase on 3 and 4 byte utf8 sequences
        @todo add base64
*/
namespace text {

inline std::wstring &convert(const std::string &utf8, std::wstring &wide) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wideconv;

  wide = wideconv.from_bytes(utf8);
  return wide;
}

inline std::string &convert(const std::wstring &wide, std::string &utf8) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wideconv;

  utf8 = wideconv.to_bytes(wide);
  return utf8;
}

inline std::wstring &tolower(const std::wstring &mixed, std::wstring &lower) {
  std::locale utf8Locale("en_US.UTF-8");

  lower.reserve(lower.size() + mixed.size());
  for (auto c = mixed.begin(); c != mixed.end(); ++c) {
    lower.append(1, std::tolower(*c, utf8Locale));
  }
  return lower;
}

inline std::string &tolower(const std::string &mixed, std::string &lower) {
  std::wstring wmixed, wlower;

  return convert(tolower(convert(mixed, wmixed), wlower), lower);
}

inline std::string tolower(const std::string &mixed) {
  std::string buffer;

  return tolower(mixed, buffer);
}

inline std::string &toHex(const std::string &binary, std::string &hex) {
  const char *const hexDigits = "0123456789abcdef";

  for (int i = 0; (i < static_cast<int>(binary.size())); ++i) {
    const int lowerIndex = (binary[i] >> 4) & 0x0F;
    const int upperIndex = binary[i] & 0x0F;

    hex.append(1, hexDigits[lowerIndex]);
    hex.append(1, hexDigits[upperIndex]);
  }
  return hex;
}

inline std::string &toHex(const std::string &binary) {
  std::string buffer;

  return toHex(binary, buffer);
}

inline std::string &fromHex(const std::string &hex, std::string &binary) {
  std::string hexDigits("0123456789abcdef");

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

inline std::string &fromHex(const std::string &hex) {
  std::string buffer;

  return fromHex(hex, buffer);
}

} // namespace text

#endif // __Text_h__

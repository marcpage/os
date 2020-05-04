#ifndef __Text_h__
#define __Text_h__

#include <codecvt>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>

/**
	@todo Get it to work for non-Roman letters
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
  lower.reserve(lower.size() + mixed.size());
  for (auto c = mixed.begin(); c != mixed.end(); ++c) {
    lower.append(1, towlower(*c));
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

} // namespace text

#endif // __Text_h__

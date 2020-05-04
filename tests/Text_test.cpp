#include "os/Text.h"
#include <stdio.h>

#define dotest(condition)                                                      \
  if (!(condition)) {                                                          \
    fprintf(stderr, "FAIL(%s:%d): %s\n", __FILE__, __LINE__, #condition);      \
  }

int main(const int /*argc*/, const char *const /*argv*/[]) {
  int iterations = 2;
#ifdef __Tracer_h__
  iterations = 1;
#endif
  const char *strings[] = {
      "test",     "test",     // no change
      "Test",     "test",     // first character
      "TEST",     "test",     // all characters
      "\xd0\xac", "\xd1\x8c", // utf8
      "\xd0\xad", "\xd1\x8d", // utf8
      "\xd0\xae", "\xd1\x8e", // utf8
      "\xd0\xaf", "\xd1\x8f", // utf8
      "\xd1\xa0", "\xd1\xa1", // utf8
      "\xd1\xa2", "\xd1\xa3", // utf8
      "\xd1\xa4", "\xd1\xa5", // utf8
      "\xd1\xa6", "\xd1\xa7", // utf8
      "\xd1\xa8", "\xd1\xa9", // utf8
      "\xd1\xaa", "\xd1\xab", // utf8
      "\xd1\xac", "\xd1\xad", // utf8
      "\xd1\xae", "\xd1\xaf", // utf8
      "\xd1\xb0", "\xd1\xb1", // utf8
      "\xd1\xb2", "\xd1\xb3", // utf8
      "\xd1\xb4", "\xd1\xb5", // utf8
      "\xd1\xb6", "\xd1\xb7", // utf8
      "\xd1\xb8", "\xd1\xb9", // utf8
      "\xd1\xba", "\xd1\xbb", // utf8
      "\xd1\xbc", "\xd1\xbd", // utf8
      "\xd1\xbe", "\xd1\xbf", // utf8
      "\xd2\x80", "\xd2\x81", // utf8
      "\xd2\x8a", "\xd2\x8b", // utf8
      "\xd2\x8c", "\xd2\x8d", // utf8
      "\xd2\x8e", "\xd2\x8f", // utf8
      "\xd2\x90", "\xd2\x91", // utf8
      "\xd2\x92", "\xd2\x93", // utf8
      "\xd2\x94", "\xd2\x95", // utf8
      "\xd2\x96", "\xd2\x97", // utf8
      "\xd2\x98", "\xd2\x99", // utf8
      "\xd2\x9a", "\xd2\x9b", // utf8
      "\xd2\x9c", "\xd2\x9d", // utf8
      "\xd2\x9e", "\xd2\x9f", // utf8
      "\xd2\xa0", "\xd2\xa1", // utf8
      "\xd2\xa2", "\xd2\xa3", // utf8
      "\xc3\x93", "\xc3\xb3", // utf8
      "\xc3\x89", "\xc3\xa9", // utf8
      "\xc3\x9c", "\xc3\xbc", // utf8
      "\xce\xa3", "\xcf\x83", // utf8
      "\xc6\x90", "\xc9\x9b", // utf8
  };
  const struct {
    const char *text;
    text::Base64Style style;
    int split;
    const char *eol;
    const char *expected;
  } encode64[] = {
      {"te", text::Base64, 0, "", "dGU="},                       // bug
      {">>>???", text::Base64, 0, "", "Pj4+Pz8/"},               // url special
      {">>>???", text::Base64URL, 0, "", "Pj4-Pz8_"},            // url special
      {"Test", text::Base64, 0, "", "VGVzdA=="},                 // two padding
      {"Run", text::Base64, 0, "", "UnVu"},                      // no padding
      {"Waste", text::Base64, 0, "", "V2FzdGU="},                // one padding
      {"Test", text::Base64URL, 0, "", "VGVzdA.."},              // two padding
      {"Run", text::Base64URL, 0, "", "UnVu"},                   // no padding
      {"Waste", text::Base64URL, 0, "", "V2FzdGU."},             // one padding
      {"Test", text::Base64, 3, "\r\n", "VGV\r\nzdA\r\n=="},     // two padding
      {"Run", text::Base64, 3, "\r\n", "UnV\r\nu"},              // no padding
      {"Waste", text::Base64, 3, "\r\n", "V2F\r\nzdG\r\nU="},    // one padding
      {"Test", text::Base64URL, 3, "\r\n", "VGV\r\nzdA\r\n.."},  // two padding
      {"Run", text::Base64URL, 3, "\r\n", "UnV\r\nu"},           // no padding
      {"Waste", text::Base64URL, 3, "\r\n", "V2F\r\nzdG\r\nU."}, // one padding
      {"Test", text::Base64, 3, "\n", "VGV\nzdA\n=="},           // two padding
      {"Run", text::Base64, 3, "\n", "UnV\nu"},                  // no padding
      {"Waste", text::Base64, 3, "\n", "V2F\nzdG\nU="},          // one padding
      {"Test", text::Base64URL, 3, "\n", "VGV\nzdA\n.."},        // two padding
      {"Run", text::Base64URL, 3, "\n", "UnV\nu"},               // no padding
      {"Waste", text::Base64URL, 3, "\n", "V2F\nzdG\nU."},       // one padding
  };
  for (int i = 0; i < iterations; ++i) {
    try {
      for (int j = 0; j < int(sizeof(strings) / sizeof(strings[0]) / 2); ++j) {
        std::string mixed = strings[2 * j];
        std::string lower = strings[2 * j + 1];

        if (text::tolower(mixed) != lower) {
          printf("FAIL: '%s' -> '%s' but got '%s'\n", mixed.c_str(),
                 lower.c_str(), text::tolower(mixed).c_str());
        }
      }
      for (int j = 0; j < int(sizeof(encode64) / sizeof(encode64[0])); ++j) {
        const auto &entry = encode64[j];
        std::string encoded =
            text::base64Encode(entry.text, entry.style, entry.split, entry.eol);
        std::string decoded = text::base64Decode(encoded);

        if (encoded != entry.expected) {
          printf("FAIL: '%s' -> '%s' but got '%s'\n", entry.text,
                 entry.expected, encoded.c_str());
        }
        if (decoded != entry.text) {
          printf("FAIL: '%s' <- '%s' but got '%s'\n", entry.text,
                 entry.expected, decoded.c_str());
        }
      }
    } catch (const std::exception &exception) {
      printf("FAIL: Exception not caught: %s\n", exception.what());
    }
  }
  return 0;
}

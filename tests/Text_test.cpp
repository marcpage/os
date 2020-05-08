#include "os/Text.h"
#include <stdio.h>

#define dotest(condition)                                                      \
  if (!(condition)) {                                                          \
    fprintf(stderr, "FAIL(%s:%d): %s\n", __FILE__, __LINE__, #condition);      \
  }

int main(const int /*argc*/, const char *const /*argv*/[]) {
  int iterations = 200;
#ifdef __Tracer_h__
  iterations = 1;
#endif
  const char *strings[] = {
      "test",     "test",     // no change
      "Test",     "test",     // first character
      "TEST",     "test",     // all characters
      "\x41",     "\x61",     // ascii
      "\x42",     "\x62",     // ascii
      "\x43",     "\x63",     // ascii
      "\x44",     "\x64",     // ascii
      "\x45",     "\x65",     // ascii
      "\x46",     "\x66",     // ascii
      "\x47",     "\x67",     // ascii
      "\x48",     "\x68",     // ascii
      "\x49",     "\x69",     // ascii
      "\x4a",     "\x6a",     // ascii
      "\x4b",     "\x6b",     // ascii
      "\x4c",     "\x6c",     // ascii
      "\x4d",     "\x6d",     // ascii
      "\x4e",     "\x6e",     // ascii
      "\x4f",     "\x6f",     // ascii
      "\x50",     "\x70",     // ascii
      "\x51",     "\x71",     // ascii
      "\x52",     "\x72",     // ascii
      "\x53",     "\x73",     // ascii
      "\x54",     "\x74",     // ascii
      "\x55",     "\x75",     // ascii
      "\x56",     "\x76",     // ascii
      "\x57",     "\x77",     // ascii
      "\x58",     "\x78",     // ascii
      "\x59",     "\x79",     // ascii
      "\x5a",     "\x7a",     // ascii
      "\xc3\x84", "\xc3\xa4", // utf8
      "\xc3\x89", "\xc3\xa9", // utf8
      "\xc3\x93", "\xc3\xb3", // utf8
      "\xc3\x96", "\xc3\xb6", // utf8
      "\xc3\x9c", "\xc3\xbc", // utf8
      "\xc6\x90", "\xc9\x9b", // utf8
      "\xce\x91", "\xce\xb1", // utf8
      "\xce\x92", "\xce\xb2", // utf8
      "\xce\x93", "\xce\xb3", // utf8
      "\xce\x94", "\xce\xb4", // utf8
      "\xce\x95", "\xce\xb5", // utf8
      "\xce\x96", "\xce\xb6", // utf8
      "\xce\x97", "\xce\xb7", // utf8
      "\xce\x98", "\xce\xb8", // utf8
      "\xce\x99", "\xce\xb9", // utf8
      "\xce\x9a", "\xce\xba", // utf8
      "\xce\x9b", "\xce\xbb", // utf8
      "\xce\x9c", "\xce\xbc", // utf8
      "\xce\x9d", "\xce\xbd", // utf8
      "\xce\x9e", "\xce\xbe", // utf8
      "\xce\x9f", "\xce\xbf", // utf8
      "\xce\xa0", "\xcf\x80", // utf8
      "\xce\xa1", "\xcf\x81", // utf8
      "\xce\xa3", "\xcf\x83", // utf8
      "\xce\xa4", "\xcf\x84", // utf8
      "\xce\xa5", "\xcf\x85", // utf8
      "\xce\xa6", "\xcf\x86", // utf8
      "\xce\xa7", "\xcf\x87", // utf8
      "\xce\xa8", "\xcf\x88", // utf8
      "\xce\xa9", "\xcf\x89", // utf8
      "\xd0\x81", "\xd1\x91", // utf8
      "\xd0\x86", "\xd1\x96", // utf8
      "\xd0\x90", "\xd0\xb0", // utf8
      "\xd0\x91", "\xd0\xb1", // utf8
      "\xd0\x92", "\xd0\xb2", // utf8
      "\xd0\x93", "\xd0\xb3", // utf8
      "\xd0\x94", "\xd0\xb4", // utf8
      "\xd0\x95", "\xd0\xb5", // utf8
      "\xd0\x96", "\xd0\xb6", // utf8
      "\xd0\x97", "\xd0\xb7", // utf8
      "\xd0\x98", "\xd0\xb8", // utf8
      "\xd0\x99", "\xd0\xb9", // utf8
      "\xd0\x9a", "\xd0\xba", // utf8
      "\xd0\x9b", "\xd0\xbb", // utf8
      "\xd0\x9c", "\xd0\xbc", // utf8
      "\xd0\x9d", "\xd0\xbd", // utf8
      "\xd0\x9e", "\xd0\xbe", // utf8
      "\xd0\x9f", "\xd0\xbf", // utf8
      "\xd0\xa0", "\xd1\x80", // utf8
      "\xd0\xa1", "\xd1\x81", // utf8
      "\xd0\xa2", "\xd1\x82", // utf8
      "\xd0\xa3", "\xd1\x83", // utf8
      "\xd0\xa4", "\xd1\x84", // utf8
      "\xd0\xa5", "\xd1\x85", // utf8
      "\xd0\xa6", "\xd1\x86", // utf8
      "\xd0\xa7", "\xd1\x87", // utf8
      "\xd0\xa8", "\xd1\x88", // utf8
      "\xd0\xa9", "\xd1\x89", // utf8
      "\xd0\xaa", "\xd1\x8a", // utf8
      "\xd0\xab", "\xd1\x8b", // utf8
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
  const char *base64[] = {
      "dGU=",
      "te", // single padding

      "dGU",
      "te", // missing single padding

      "VGVzdA==",
      "Test", // double padding

      "VGVzdA=",
      "Test", // double padding, missing one

      "VGVzdA",
      "Test", // double padding, missing both

      "dGU.",
      "te", // single padding

      "VGVzdA..",
      "Test", // double padding

      "VGVzdA.=",
      "Test", // double padding

      "VGVzdA=.",
      "Test", // double padding

      "VGVzdA.",
      "Test", // double padding, missing one

      "V G\rV\tz\nd  A =\t=   ",
      "Test", // whitespace
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
      for (int j = 0; j < int(sizeof(base64) / sizeof(base64[0]) / 2); ++j) {
        std::string b64 = base64[2 * j];
        std::string value = base64[2 * j + 1];

        if (text::base64Decode(b64) != value) {
          printf("FAIL: '%s' -> '%s' but got '%s'\n", b64.c_str(),
                 value.c_str(), text::base64Decode(b64).c_str());
        }
      }

    } catch (const std::exception &exception) {
      printf("FAIL: Exception not caught: %s\n", exception.what());
    }
  }
  return 0;
}

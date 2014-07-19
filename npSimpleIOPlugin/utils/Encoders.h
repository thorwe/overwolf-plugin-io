/*
  Simple IO Plugin
  Copyright (c) 2014 Overwolf Ltd.
*/
#ifndef UTILS_ENCODERS_H_
#define UTILS_ENCODERS_H_

#include <string>

namespace utils {

class Encoders {
public:
  // Convert a wide Unicode string to an UTF8 string
  static std::string utf8_encode(const std::wstring& wstr);

  // Convert an UTF8 string to a wide Unicode String
  static std::wstring utf8_decode(const std::string& str);

}; // class Encoders

}; // namespace utils;

#endif // UTILS_ENCODERS_H_
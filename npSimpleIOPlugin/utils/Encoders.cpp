/*
  Simple IO Plugin
  Copyright (c) 2014 Overwolf Ltd.
*/
#include "Encoders.h"

#include <windows.h>

using namespace utils;

// Convert a wide Unicode string to an UTF8 string
// static
std::string Encoders::utf8_encode(const std::wstring &wstr) {
  int size_needed = WideCharToMultiByte(
    CP_UTF8, 0, &wstr[0], (int)wstr.size(), nullptr, 0, nullptr, nullptr);

  std::string strTo( size_needed, 0 );
  WideCharToMultiByte(
    CP_UTF8, 
    0, 
    &wstr[0], 
    (int)wstr.size(), 
    &strTo[0], 
    size_needed, 
    nullptr, 
    nullptr);
  return strTo;
}

// Convert an UTF8 string to a wide Unicode String
// static
std::wstring Encoders::utf8_decode(const std::string &str) {
  int size_needed = MultiByteToWideChar(
    CP_UTF8, 0, &str[0], (int)str.size(), nullptr, 0);

  std::wstring wstrTo( size_needed, 0 );
  MultiByteToWideChar(
    CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
  return wstrTo;
}

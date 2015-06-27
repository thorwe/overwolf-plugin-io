/*
  Simple IO Plugin
  Copyright (c) 2014 Overwolf Ltd.
*/
#ifndef UTILS_FILE_H_
#define UTILS_FILE_H_

#include <string>
#include <shlobj.h>

namespace utils {

class File {
public:
	static std::string File::GetLastErrorStdStr();
  static std::wstring GetSpecialFolderWide(int csidl);
  static std::string GetSpecialFolderUtf8(int csidl);

  static bool DoesFileExist(const std::wstring& filename);
  static bool IsDirectory(const std::wstring& directory);

  static std::string GetTextFile(
    const std::wstring& filename, 
    std::string& ref_output,
    int limit);

  static bool GetFileTimes(
    const std::wstring& filename, 
    __int64& ref_creation_time,
    __int64& ref_last_access_time,
    __int64& ref_last_write_time);

  static std::string SetFile(
	  const std::wstring& filename,
	  const std::string& ref_input
	  );

  static bool ListDirectoryContents(
	  const std::wstring& filename,
	  std::string& ref_output
	  );

}; // class File

}; // namespace utils;

#endif // UTILS_FILE_H_
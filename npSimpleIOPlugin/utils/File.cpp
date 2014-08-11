/*
  Simple IO Plugin
  Copyright (c) 2014 Overwolf Ltd.
*/
#include "File.h"
#include "Encoders.h"

#include <windows.h>
#include <shlwapi.h>

using namespace utils;

// static
std::wstring File::GetSpecialFolderWide(int csidl) {
  WCHAR szPath[MAX_PATH] = {0};
  if (0 > SHGetFolderPathW(
    nullptr, csidl, nullptr, SHGFP_TYPE_CURRENT, szPath)) {
    return L"";
  }

  return szPath;
}

// static 
std::string File::GetSpecialFolderUtf8(int csidl) {
  return Encoders::utf8_encode(File::GetSpecialFolderWide(csidl));
}

// static 
bool File::DoesFileExist(const std::wstring& filename) {
  DWORD dwAttributes = GetFileAttributesW(filename.c_str());
  return (INVALID_FILE_ATTRIBUTES != dwAttributes);
}

// static 
bool File::IsDirectory(const std::wstring& directory) {
  DWORD dwAttributes = GetFileAttributesW(directory.c_str());

  if (INVALID_FILE_ATTRIBUTES == dwAttributes) {
    return false;
  }

  return ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 
            FILE_ATTRIBUTE_DIRECTORY);
}

// static
bool File::GetTextFile(
  const std::wstring& filename,
  std::string& ref_output,
  int limit) {
  
  DWORD dwSize = MAX_PATH;
  WCHAR path[MAX_PATH] = {NULL};
  if (0 >= GetTempPathW(dwSize, path)) {
    return false;
  }

  WCHAR temp_file[MAX_PATH] = {NULL};
  if (0 == GetTempFileNameW(path, L"IO_", 0, temp_file)) {
    return false;
  }

  if (FALSE == CopyFileW(filename.c_str(), temp_file, FALSE)) {
    return false;
  }

  ref_output.clear();
 
  HANDLE hFile = CreateFileW(
    temp_file,
    GENERIC_READ,          // open for reading
    FILE_SHARE_READ,       // share for reading
    NULL,                  // default security
    OPEN_EXISTING,         // existing file only
    FILE_ATTRIBUTE_NORMAL, // normal file
    NULL);                 // no attr. template

  if (INVALID_HANDLE_VALUE == hFile) {
    return false;
  }

  bool status = false;
  dwSize = GetFileSize(hFile, NULL);

  if (dwSize > 0) {
    if (limit > 0) {
      dwSize = limit;
    }
    char* buffer = new char[dwSize];

    status = (TRUE == ReadFile(
      hFile, 
      (void*)buffer, 
      dwSize, 
      nullptr, 
      nullptr));

    if (status) {
      ref_output.insert(0, buffer, dwSize);
    }

    delete[] buffer;
  }

  CloseHandle(hFile);

  return status;
}
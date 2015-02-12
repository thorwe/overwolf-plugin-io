/*
  Simple IO Plugin
  Copyright (c) 2014 Overwolf Ltd.
*/
#include "File.h"
#include "Encoders.h"
#include "Base64.h"

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

    DWORD dwBytesReadDummy = 0; // otherwise we get a crash in win7 (a.k.a. RTFM)

    status = (TRUE == ReadFile(
      hFile, 
      (void*)buffer, 
      dwSize, 
      &dwBytesReadDummy, 
      nullptr));

    if (status) {
      ref_output.insert(0, buffer, dwSize);
    }

    delete[] buffer;
  }

  CloseHandle(hFile);

  return status;
}

//static 
bool File::GetFileTimes(
  const std::wstring& filename, 
  __int64& ref_creation_time,
  __int64& ref_last_access_time,
  __int64& ref_last_write_time) {
  return false;
}


bool File::SetFile(
	const std::wstring& filename,
	const std::string& ref_input
	) {

	std::vector<BYTE> DataBuffer;

	// automagical Base64 encoded png support

	size_t filename_size = filename.size();	// wstring size, good enough for last chars?
	std::wstring file_ending;

	if (filename_size > 4)
	{
		file_ending = filename.substr(filename_size - 4, 4);
	};

	if (file_ending == L".png" || file_ending == L".PNG")	// good enough for me
	{
		std::size_t pos = ref_input.find("data:image/png;base64,");	
		if (pos != std::string::npos)
		{
			DataBuffer = base64_decode(ref_input.substr(pos + 22));
		}
	}
	else
	{
		DataBuffer.assign((byte*)ref_input.c_str(), (byte*)ref_input.c_str() + strlen(ref_input.c_str()));
	}

	DWORD dwBytesToWrite = DataBuffer.size();
	if (dwBytesToWrite == 0)
		return false;

	// prefix "\\?\" for http://msdn.microsoft.com/en-us/library/windows/desktop/aa363858%28v=vs.85%29.aspx
	std::wstring ufname = L"\\\\?\\" + filename;

	LPCWSTR f_name = (LPCWSTR)ufname.c_str();
	HANDLE hFile = CreateFileW(
		f_name,
		GENERIC_WRITE,          // open for writing
		0,                      // do not share
		NULL,                   // default security
		CREATE_ALWAYS,             // overwrite existing file
		FILE_ATTRIBUTE_NORMAL,  // normal file
		NULL);                  // no attr. template

	if (INVALID_HANDLE_VALUE == hFile) {
		return false;
	}

	bool status = false;
	DWORD dwBytesWritten = 0;
	BOOL bErrorFlag = FALSE;

	bErrorFlag = WriteFile(
		hFile,           // open file handle
		DataBuffer.data(),      // start of data to write
		dwBytesToWrite,  // number of bytes to write
		&dwBytesWritten, // number of bytes that were written
		NULL);            // no overlapped structure

	if (FALSE != bErrorFlag)
	{
		if (dwBytesWritten == dwBytesToWrite)
		{
			status = true;
		}
		/*else
		{
		// This is an error because a synchronous write that results in
		// success (WriteFile returns TRUE) should write all data as
		// requested. This would not necessarily be the case for
		// asynchronous writes.
		}*/
	}

	CloseHandle(hFile);

	return status;
}

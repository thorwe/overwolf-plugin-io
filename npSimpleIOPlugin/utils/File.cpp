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

std::string File::GetLastErrorStdStr()
{
	DWORD error = GetLastError();
	if (error)
	{
		LPVOID lpMsgBuf;
		DWORD bufLen = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);
		if (bufLen)
		{
			LPCSTR lpMsgStr = (LPCSTR)lpMsgBuf;
			std::string result(lpMsgStr, lpMsgStr + bufLen);

			LocalFree(lpMsgBuf);

			return result;
		}
	}
	return std::string();
}

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
std::string File::GetTextFile(
  const std::wstring& filename,
  std::string& ref_output,
  int limit) {
  
	std::string status = "indetermined";

  DWORD dwSize = MAX_PATH;
  WCHAR path[MAX_PATH] = {NULL};
  if (0 >= GetTempPathW(dwSize, path)) {
	  status = "Couldn't get temp path.";
	  return status.append(utils::File::GetLastErrorStdStr());
  }

  WCHAR temp_file[MAX_PATH] = {NULL};
  if (0 == GetTempFileNameW(path, L"IOW", 0, temp_file)) {
    status = "Couldn't get temp file name:";
	return status.append(utils::File::GetLastErrorStdStr());
  }

  std::wstring temp_file_full = L"\\\\?\\";
  temp_file_full.append(temp_file);

  if (FALSE == CopyFileW(filename.c_str(), temp_file_full.c_str(), FALSE)) {
    status = "Couldn't copy to temp file:";
	return status.append(utils::File::GetLastErrorStdStr());
  }

  ref_output.clear();
 
  HANDLE hFile = CreateFileW(
    temp_file_full.c_str(),
    GENERIC_READ,          // open for reading
    FILE_SHARE_READ,       // share for reading
    NULL,                  // default security
    OPEN_EXISTING,         // existing file only
    FILE_ATTRIBUTE_NORMAL, // normal file
    NULL);                 // no attr. template

  if (INVALID_HANDLE_VALUE == hFile) {
	  status = "invalid file handle:";
	  return status.append(utils::File::GetLastErrorStdStr());
  }

  dwSize = GetFileSize(hFile, NULL);

  if (dwSize > 0) {
    if (limit > 0) {
      dwSize = limit;
    }
    char* buffer = new char[dwSize];

    DWORD dwBytesReadDummy = 0; // otherwise we get a crash in win7 (a.k.a. RTFM)

	if (TRUE == ReadFile(
		hFile,
		(void*)buffer,
		dwSize,
		&dwBytesReadDummy,
		nullptr)) {
		ref_output.insert(0, buffer, dwSize);
		status = "success";
	}

    delete[] buffer;
  }

  CloseHandle(hFile);

  if ((DeleteFileW(temp_file_full.c_str())) == 0) {
	  status = "Read success, but couldn't delete temp file:";
	  status.append(utils::File::GetLastErrorStdStr());
  }
  
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


std::string File::SetFile(
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
	if (dwBytesToWrite == 0) {
		return "No data to write found.";
	}

	// prefix "\\?\" for http://msdn.microsoft.com/en-us/library/windows/desktop/aa363858%28v=vs.85%29.aspx
	std::wstring ufname = L"\\\\?\\" + filename;

	LPCWSTR f_name = (LPCWSTR)ufname.c_str();
	HANDLE hFile = CreateFileW(
		f_name,
		GENERIC_READ | GENERIC_WRITE,	// open for writing; adding READ fixes mapped network drive problems
		FILE_SHARE_READ,				// do not share can cause troubles with antivirus, search indexing etc.
		NULL,							// default security
		CREATE_ALWAYS,					// overwrite existing file
		FILE_ATTRIBUTE_NORMAL,			// normal file
		NULL);							// no attr. template

	std::string status = "indetermined";
	if (INVALID_HANDLE_VALUE == hFile) {
		status = "invalid file handle:";
		return status.append(utils::File::GetLastErrorStdStr());
	}

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
			status = "success";
		}
		else
		{
			// This is an error because a synchronous write that results in
			// success (WriteFile returns TRUE) should write all data as
			// requested. This would not necessarily be the case for
			// asynchronous writes.
			status = "not all bytes could be written in synchronous operation.";
		}
	}
	else
	{
		status = "Error writing file:";
		status.append(utils::File::GetLastErrorStdStr());
	}

	CloseHandle(hFile);

	return status;
}

bool File::ListDirectoryContents(
	const std::wstring& pathname,
	std::string& ref_output) {

	if (!IsDirectory(pathname))
		return false;

	WIN32_FIND_DATAW fdFile;
	HANDLE hFind = NULL;

	wchar_t sPath[2048];
	
	//Specify a file mask. *.* = We want everything! 
	wsprintfW(sPath, L"%s\\*.*", pathname.c_str());
	
	std::wstring sFolders = L"";
	std::wstring sFiles = L"";

	if ((hFind = FindFirstFileW(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
		return false;

	do
	{
		//Find first file will always return "."
		//    and ".." as the first two directories. 
		if (wcscmp(fdFile.cFileName, L".") != 0
			&& wcscmp(fdFile.cFileName, L"..") != 0)
		{
			if (fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
			{
				sFolders += L"\"";
				sFolders += fdFile.cFileName;
				sFolders += (L"\",");
			}
			else
			{
				sFiles += L"\"";
				sFiles += fdFile.cFileName;
				sFiles += (L"\",");
			}

		}
	} while (FindNextFileW(hFind, &fdFile)); //Find the next file. 

	FindClose(hFind); //Always, Always, clean things up! 

	std::wstring sOut = L"{\"files\":[";
	
	if (!sFiles.empty())
	{
		sOut.append(sFiles);
		sOut.pop_back();	// remove last L","
	}
	sOut.append(L"],\"folders\":[");
	if (!sFolders.empty())
	{
		sOut.append(sFolders);
		sOut.pop_back();	// remove last L","
	}
	sOut.append(L"]}");
	
	ref_output = Encoders::utf8_encode(sOut);

	return true;
}
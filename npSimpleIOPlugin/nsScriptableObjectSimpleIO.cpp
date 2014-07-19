/*
  Simple IO Plugin
  Copyright (c) 2014 Overwolf Ltd.
*/
#include "nsScriptableObjectSimpleIO.h"
#include "utils/Thread.h"
#include "utils/File.h"
#include "utils/Encoders.h"

#include <sstream>

#define REGISTER_METHOD(name, method) { \
  methods_[NPN_GetStringIdentifier(name)] = &method; \
}

#define REGISTER_GET_PROPERTY(name, val) { \
  properties_[NPN_GetStringIdentifier(name)] = val; \
}

nsScriptableObjectSimpleIO::nsScriptableObjectSimpleIO(NPP npp) :
  nsScriptableObjectBase(npp),
  shutting_down_(false) {
}

nsScriptableObjectSimpleIO::~nsScriptableObjectSimpleIO(void) {
  shutting_down_ = true;
  
  if (thread_.get()) {
    thread_->Stop();
  }
}

bool nsScriptableObjectSimpleIO::Init() {
  REGISTER_METHOD(
    "fileExists", 
    nsScriptableObjectSimpleIO::FileExists);
  REGISTER_METHOD(
    "isDirectory", 
    nsScriptableObjectSimpleIO::IsDirectory);
  REGISTER_METHOD(
    "getTextFile", 
    nsScriptableObjectSimpleIO::GetTextFile);
  REGISTER_METHOD(
    "getBinaryFile", 
    nsScriptableObjectSimpleIO::GetBinaryFile);

  REGISTER_GET_PROPERTY(
    "PROGRAMFILES", 
    utils::File::GetSpecialFolderUtf8(CSIDL_PROGRAM_FILES));
  REGISTER_GET_PROPERTY(
    "PROGRAMFILESX86", 
    utils::File::GetSpecialFolderUtf8(CSIDL_PROGRAM_FILESX86));
  REGISTER_GET_PROPERTY(
    "COMMONFILES", 
    utils::File::GetSpecialFolderUtf8(CSIDL_PROGRAM_FILES_COMMON));
  REGISTER_GET_PROPERTY(
    "COMMONFILESX86", 
    utils::File::GetSpecialFolderUtf8(CSIDL_PROGRAM_FILES_COMMONX86));
  REGISTER_GET_PROPERTY(
    "COMMONAPPDATA", 
    utils::File::GetSpecialFolderUtf8(CSIDL_COMMON_APPDATA));
  REGISTER_GET_PROPERTY(
    "DESKTOP", 
    utils::File::GetSpecialFolderUtf8(CSIDL_DESKTOP));
  REGISTER_GET_PROPERTY(
    "WINDIR", 
    utils::File::GetSpecialFolderUtf8(CSIDL_WINDOWS));
  REGISTER_GET_PROPERTY(
    "SYSDIR", 
    utils::File::GetSpecialFolderUtf8(CSIDL_SYSTEM));
  REGISTER_GET_PROPERTY(
    "SYSDIRX86", 
    utils::File::GetSpecialFolderUtf8(CSIDL_SYSTEMX86));


  thread_.reset(new utils::Thread());
  return thread_->Start();
}

bool nsScriptableObjectSimpleIO::HasMethod(NPIdentifier name) {
#ifdef _DEBUG
  NPUTF8* name_utf8 = NPN_UTF8FromIdentifier(name);
  NPN_MemFree((void*)name_utf8);
#endif

  // does the method exist?
  return (methods_.find(name) != methods_.end());
}

bool nsScriptableObjectSimpleIO::Invoke(
  NPIdentifier name, 
  const NPVariant *args, 
  uint32_t argCount, 
  NPVariant *result) {
#ifdef _DEBUG
      NPUTF8* szName = NPN_UTF8FromIdentifier(name);
      NPN_MemFree((void*)szName);
#endif

  // dispatch method to appropriate handler
  MethodsMap::iterator iter = methods_.find(name);
  
  if (iter == methods_.end()) {
    // should never reach here
    NPN_SetException(this, "bad function called??");
    return false;
  }

  return (this->*iter->second)(name, args, argCount, result);
}

/************************************************************************/
/* Public properties
/************************************************************************/
bool nsScriptableObjectSimpleIO::HasProperty(NPIdentifier name) {
#ifdef _DEBUG
  NPUTF8* name_utf8 = NPN_UTF8FromIdentifier(name);
  NPN_MemFree((void*)name_utf8);
#endif

  // does the property exist?
  return (properties_.find(name) != properties_.end());
}

bool nsScriptableObjectSimpleIO::GetProperty(
  NPIdentifier name, NPVariant *result) {

  PropertiesMap::iterator iter = properties_.find(name);
  if (iter == properties_.end()) {
    NPN_SetException(this, "unknown property!?");
    return true;
  }

  char *resultString = (char*)NPN_MemAlloc(iter->second.size());
  memcpy(
    resultString, 
    iter->second.c_str(), 
    iter->second.size());

  STRINGN_TO_NPVARIANT(resultString, iter->second.size(), *result);

  return true;
}

bool nsScriptableObjectSimpleIO::SetProperty(
  NPIdentifier name, const NPVariant *value) {
  NPN_SetException(this, "this property is read-only!");
  return true;
}


/************************************************************************/
/* Public methods
/************************************************************************/
bool nsScriptableObjectSimpleIO::FileExists(
  NPIdentifier name, 
  const NPVariant *args, 
  uint32_t argCount, 
  NPVariant *result) {

  if (argCount < 2 ||
    !NPVARIANT_IS_STRING(args[0]) ||
    !NPVARIANT_IS_OBJECT(args[1])) {
      NPN_SetException(this, "invalid params passed to function");
      return true;
  }

  // add ref count to callback object so it won't delete
  NPN_RetainObject(NPVARIANT_TO_OBJECT(args[1]));

  // convert into std::string
  std::string filename;
  filename.append(
    NPVARIANT_TO_STRING(args[0]).UTF8Characters,
    NPVARIANT_TO_STRING(args[0]).UTF8Length);

  // post to separate thread so that we are responsive
  return thread_->PostTask(
    std::bind(
      &nsScriptableObjectSimpleIO::FileExistsTask, 
      this, 
      filename, 
      false,
      NPVARIANT_TO_OBJECT(args[1])));
}

bool nsScriptableObjectSimpleIO::IsDirectory(
  NPIdentifier name, 
  const NPVariant *args, 
  uint32_t argCount, 
  NPVariant *result) {

  if (argCount < 2 ||
    !NPVARIANT_IS_STRING(args[0]) ||
    !NPVARIANT_IS_OBJECT(args[1])) {
      NPN_SetException(this, "invalid params passed to function");
      return true;
  }

  // add ref count to callback object so it won't delete
  NPN_RetainObject(NPVARIANT_TO_OBJECT(args[1]));

  // convert into std::string
  std::string filename;
  filename.append(
    NPVARIANT_TO_STRING(args[0]).UTF8Characters,
    NPVARIANT_TO_STRING(args[0]).UTF8Length);

  // post to separate thread so that we are responsive
  return thread_->PostTask(
    std::bind(
      &nsScriptableObjectSimpleIO::FileExistsTask, 
      this, 
      filename, 
      true,
      NPVARIANT_TO_OBJECT(args[1])));
}

bool nsScriptableObjectSimpleIO::GetTextFile(
  NPIdentifier name, 
  const NPVariant *args, 
  uint32_t argCount, 
  NPVariant *result) {

  if (argCount < 3 ||
      !NPVARIANT_IS_STRING(args[0]) ||
      !NPVARIANT_IS_BOOLEAN(args[1]) ||
      !NPVARIANT_IS_OBJECT(args[2])) {
    NPN_SetException(this, "invalid params passed to function");
    return true;
  }

  // add ref count to callback object so it won't delete
  NPN_RetainObject(NPVARIANT_TO_OBJECT(args[2]));

  std::string filename;
  filename.append(
    NPVARIANT_TO_STRING(args[0]).UTF8Characters,
    NPVARIANT_TO_STRING(args[0]).UTF8Length);

  // post to separate thread so that we are responsive
  return thread_->PostTask(
    std::bind(
      &nsScriptableObjectSimpleIO::GetTextFileTask, 
      this, 
      filename, 
      NPVARIANT_TO_BOOLEAN(args[1]),
      NPVARIANT_TO_OBJECT(args[2])));
}

bool nsScriptableObjectSimpleIO::GetBinaryFile(
  NPIdentifier name, 
  const NPVariant *args, 
  uint32_t argCount, 
  NPVariant *result) {

  if (argCount < 3 ||
      !NPVARIANT_IS_STRING(args[0]) ||
      !NPVARIANT_IS_DOUBLE(args[1]) ||
      !NPVARIANT_IS_OBJECT(args[2])) {
    NPN_SetException(this, "invalid params passed to function");
    return true;
  }

  // add ref count to callback object so it won't delete
  NPN_RetainObject(NPVARIANT_TO_OBJECT(args[2]));

  std::string filename;
  filename.append(
    NPVARIANT_TO_STRING(args[0]).UTF8Characters,
    NPVARIANT_TO_STRING(args[0]).UTF8Length);


  int limit = (int)NPVARIANT_TO_DOUBLE(args[1]);

  // post to separate thread so that we are responsive
  return thread_->PostTask(
    std::bind(
      &nsScriptableObjectSimpleIO::GetBinaryFileTask, 
      this, 
      filename, 
      limit,
      NPVARIANT_TO_OBJECT(args[2])));
}

/************************************************************************/
/* Separate thread implementations for public functions
/************************************************************************/
void nsScriptableObjectSimpleIO::FileExistsTask(
  const std::string& filename, 
  bool is_directory_test, 
  NPObject* callback) {

  if (shutting_down_) {
    return;
  }

  std::wstring wide_filename = utils::Encoders::utf8_decode(filename);

  bool exists = false;

  if (is_directory_test) {
    exists = utils::File::IsDirectory(wide_filename);
  } else {
    exists = utils::File::DoesFileExist(wide_filename);
  }

  NPVariant arg;
  NPVariant ret_val;

  BOOLEAN_TO_NPVARIANT(
    exists,
    arg);

  // fire callback
  NPN_InvokeDefault(
    __super::npp_, 
    callback, 
    &arg, 
    1, 
    &ret_val);

  NPN_ReleaseVariantValue(&ret_val);
}


void nsScriptableObjectSimpleIO::GetTextFileTask(
  const std::string& filename, 
  bool widechars,
  NPObject* callback) {

  if (shutting_down_) {
    return;
  }

  std::wstring wide_filename = utils::Encoders::utf8_decode(filename);

  std::string output;
  bool status = utils::File::GetTextFile(wide_filename, output, -1);

  if (status && widechars) {
    try {
      std::wstring wstr((wchar_t*)output.c_str(), output.size()/2);
      const wchar_t* b = wstr.c_str();
      output = utils::Encoders::utf8_encode(wstr);
      const char* c = output.c_str();
    } catch(...) {

    }
  }

  NPVariant args[2];
  NPVariant ret_val;

  BOOLEAN_TO_NPVARIANT(
    status,
    args[0]);

  STRINGN_TO_NPVARIANT(
    output.c_str(),
    output.size(),
    args[1]);

  // fire callback
  NPN_InvokeDefault(
    __super::npp_, 
    callback, 
    args, 
    2, 
    &ret_val);

  NPN_ReleaseVariantValue(&ret_val);
}

void nsScriptableObjectSimpleIO::GetBinaryFileTask(
  const std::string& filename,
  int limit,
  NPObject* callback) {

  if (shutting_down_) {
    return;
  }

  std::wstring wide_filename = utils::Encoders::utf8_decode(filename);

  std::string output;
  bool status = utils::File::GetTextFile(wide_filename, output, limit);

  NPVariant args[2];
  NPVariant ret_val;

  if (status) {
    std::ostringstream str;
    if (output.size() > 0) {
      str << (int)output[0];
    }
    for (size_t i = 1; i < output.size(); ++i) {
      str << "," << (int)output[i];
    }

    output = str.str();
    STRINGN_TO_NPVARIANT(
      output.c_str(),
      output.size(),
      args[1]);
  }

  BOOLEAN_TO_NPVARIANT(
    status,
    args[0]);


  // fire callback
  NPN_InvokeDefault(
    __super::npp_, 
    callback, 
    args, 
    2, 
    &ret_val);

  NPN_ReleaseVariantValue(&ret_val);
}

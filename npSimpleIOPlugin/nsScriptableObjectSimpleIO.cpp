/*
  Simple IO Plugin
  Copyright (c) 2014 Overwolf Ltd.
*/
#include "nsScriptableObjectSimpleIO.h"
#include "utils/Thread.h"
#include "utils/File.h"

#include "plugin_method.h"
#include "plugin_method_file_exists.h"
#include "plugin_method_is_directory.h"
#include "plugin_method_get_text_file.h"
#include "plugin_method_get_binary_file.h"

#define REGISTER_METHOD(name, class) { \
  methods_[NPN_GetStringIdentifier(name)] = \
    new class(this, npp_); \
}

#define REGISTER_GET_PROPERTY(name, csidl) { \
  properties_[NPN_GetStringIdentifier(name)] = \
    utils::File::GetSpecialFolderUtf8(csidl); \
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
#pragma region public methods
  REGISTER_METHOD("fileExists", PluginMethodFileExists);
  REGISTER_METHOD("isDirectory", PluginMethodIsDirectory);
  REGISTER_METHOD("getTextFile", PluginMethodGetTextFile);
  REGISTER_METHOD("getBinaryFile", PluginMethodGetBinaryFile);
#pragma endregion public methods

#pragma region read-only properties
  REGISTER_GET_PROPERTY("PROGRAMFILES", CSIDL_PROGRAM_FILES);
  REGISTER_GET_PROPERTY("PROGRAMFILESX86", CSIDL_PROGRAM_FILESX86);
  REGISTER_GET_PROPERTY("COMMONFILES", CSIDL_PROGRAM_FILES_COMMON);
  REGISTER_GET_PROPERTY("COMMONFILESX86", CSIDL_PROGRAM_FILES_COMMONX86);
  REGISTER_GET_PROPERTY("COMMONAPPDATA", CSIDL_COMMON_APPDATA);
  REGISTER_GET_PROPERTY("DESKTOP", CSIDL_DESKTOP);
  REGISTER_GET_PROPERTY("WINDIR", CSIDL_WINDOWS);
  REGISTER_GET_PROPERTY("SYSDIR", CSIDL_SYSTEM);
  REGISTER_GET_PROPERTY("SYSDIRX86", CSIDL_SYSTEMX86);

  REGISTER_GET_PROPERTY("MYDOCUMENTS", CSIDL_MYDOCUMENTS);
  REGISTER_GET_PROPERTY("MYVIDEOS", CSIDL_MYVIDEO);
  REGISTER_GET_PROPERTY("MYPICTURES", CSIDL_MYPICTURES);
  REGISTER_GET_PROPERTY("MYMUSIC", CSIDL_MYMUSIC);
  REGISTER_GET_PROPERTY("COMMONDOCUMENTS", CSIDL_COMMON_DOCUMENTS);
  REGISTER_GET_PROPERTY("FAVORITES", CSIDL_FAVORITES);
  REGISTER_GET_PROPERTY("FONTS", CSIDL_FONTS);
  REGISTER_GET_PROPERTY("HISTORY", CSIDL_HISTORY);
  REGISTER_GET_PROPERTY("STARTMENU", CSIDL_STARTMENU);
#pragma endregion read-only properties

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

  PluginMethod* plugin_method = 
    iter->second->Clone(this, npp_, args, argCount, result);

  if (nullptr == plugin_method) {
    return false;
  }

  // post to separate thread so that we are responsive
  return thread_->PostTask(
    std::bind(
    &nsScriptableObjectSimpleIO::ExecuteMethod, 
    this,
    plugin_method));
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
/*
/************************************************************************/
void nsScriptableObjectSimpleIO::ExecuteMethod(PluginMethod* method) {
  if (shutting_down_) {
    return;
  }

  if (nullptr == method) {
    return;
  }

  method->Execute();

  if (!method->HasCallback()) {
    delete method;
    return;
  }

  NPN_PluginThreadAsyncCall(
    npp_, 
    nsScriptableObjectSimpleIO::ExecuteCallback, 
    method);
}

//static
void nsScriptableObjectSimpleIO::ExecuteCallback(void* method) {
  if (nullptr == method) {
    return;
  }

  PluginMethod* plugin_method = reinterpret_cast<PluginMethod*>(method);
  plugin_method->TriggerCallback();

  delete plugin_method;
}

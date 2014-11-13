#include "plugin_method_is_directory.h"

#include "utils/File.h"
#include "utils/Encoders.h"

// isDirectory( filename, callback(status) )
PluginMethodIsDirectory::PluginMethodIsDirectory(NPObject* object, NPP npp) : 
  PluginMethod(object, npp) {
}

//virtual 
PluginMethod* PluginMethodIsDirectory::Clone(
  NPObject* object, 
  NPP npp, 
  const NPVariant *args, 
  uint32_t argCount, 
  NPVariant *result) {

  PluginMethodIsDirectory* clone = 
    new PluginMethodIsDirectory(object, npp);

  try {
    if (argCount < 2 ||
      !NPVARIANT_IS_STRING(args[0]) ||
      !NPVARIANT_IS_OBJECT(args[1])) {
        NPN_SetException(
          __super::object_, 
          "invalid params passed to function");
        delete clone;
        return nullptr;
    }

    clone->callback_ = NPVARIANT_TO_OBJECT(args[1]);
    // add ref count to callback object so it won't delete
    NPN_RetainObject(clone->callback_);

    // convert into std::string
    clone->filename_.append(
      NPVARIANT_TO_STRING(args[0]).UTF8Characters,
      NPVARIANT_TO_STRING(args[0]).UTF8Length);

    return clone;
  } catch(...) {

  }


  delete clone;
  return nullptr;
}

// virtual
bool PluginMethodIsDirectory::HasCallback() {
  return (nullptr != callback_);
}

// virtual
void PluginMethodIsDirectory::Execute() {
  std::wstring wide_filename = utils::Encoders::utf8_decode(filename_);

  directory_ = utils::File::IsDirectory(wide_filename);
}

// virtual
void PluginMethodIsDirectory::TriggerCallback() {
  NPVariant arg;
  NPVariant ret_val;

  BOOLEAN_TO_NPVARIANT(
    directory_,
    arg);

  // fire callback
  NPN_InvokeDefault(
    __super::npp_, 
    callback_, 
    &arg, 
    1, 
    &ret_val);

  NPN_ReleaseVariantValue(&ret_val);
}
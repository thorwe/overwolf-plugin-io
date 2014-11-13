#include "plugin_method_file_exists.h"

#include "utils/File.h"
#include "utils/Encoders.h"

// fileExists( filename, callback(status) )
PluginMethodFileExists::PluginMethodFileExists(NPObject* object, NPP npp) : 
  PluginMethod(object, npp) {
}

//virtual 
PluginMethod* PluginMethodFileExists::Clone(
  NPObject* object, 
  NPP npp, 
  const NPVariant *args, 
  uint32_t argCount, 
  NPVariant *result) {

  PluginMethodFileExists* clone = 
    new PluginMethodFileExists(object, npp);

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
bool PluginMethodFileExists::HasCallback() {
  return (nullptr != callback_);
}

// virtual
void PluginMethodFileExists::Execute() {
  std::wstring wide_filename = utils::Encoders::utf8_decode(filename_);

  exists_ = utils::File::DoesFileExist(wide_filename);
}

// virtual
void PluginMethodFileExists::TriggerCallback() {
  NPVariant arg;
  NPVariant ret_val;

  BOOLEAN_TO_NPVARIANT(
    exists_,
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
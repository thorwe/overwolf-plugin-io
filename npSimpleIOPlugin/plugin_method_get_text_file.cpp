#include "plugin_method_get_text_file.h"

#include "utils/File.h"
#include "utils/Encoders.h"

// getTextFile( filename, widechar, callback(status, data) )
PluginMethodGetTextFile::PluginMethodGetTextFile(NPObject* object, NPP npp) : 
  PluginMethod(object, npp) {
}

//virtual 
PluginMethod* PluginMethodGetTextFile::Clone(
  NPObject* object, 
  NPP npp, 
  const NPVariant *args, 
  uint32_t argCount, 
  NPVariant *result) {

  PluginMethodGetTextFile* clone = 
    new PluginMethodGetTextFile(object, npp);

  try {
    if (argCount < 3 ||
      !NPVARIANT_IS_STRING(args[0]) ||
      !NPVARIANT_IS_BOOLEAN(args[1]) ||
      !NPVARIANT_IS_OBJECT(args[2])) {
      NPN_SetException(
        __super::object_, 
        "invalid params passed to function");
      delete clone;
      return nullptr;
    }

    clone->callback_ = NPVARIANT_TO_OBJECT(args[2]);
    // add ref count to callback object so it won't delete
    NPN_RetainObject(clone->callback_);

    clone->widechars_ = NPVARIANT_TO_BOOLEAN(args[1]);

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
bool PluginMethodGetTextFile::HasCallback() {
  return (nullptr != callback_);
}

// virtual
void PluginMethodGetTextFile::Execute() {
  std::wstring wide_filename = utils::Encoders::utf8_decode(filename_);

  status_ = utils::File::GetTextFile(wide_filename, output_, -1);

  if ((status_ == "success") && widechars_) {
    try {
      std::wstring wstr((wchar_t*)output_.c_str(), output_.size()/2);
      const wchar_t* b = wstr.c_str();
      output_ = utils::Encoders::utf8_encode(wstr);
      const char* debug = output_.c_str();
    } catch(...) {
      status_ = "Could not utf8_encode.";
    }
  }
}

// virtual
void PluginMethodGetTextFile::TriggerCallback() {
  NPVariant args[2];
  NPVariant ret_val;

  STRINGN_TO_NPVARIANT(
	  status_.c_str(),
	  status_.size(),
	  args[0]);

  STRINGN_TO_NPVARIANT(
    output_.c_str(),
    output_.size(),
    args[1]);

  // fire callback
  NPN_InvokeDefault(
    __super::npp_, 
    callback_, 
    args, 
    2, 
    &ret_val);

  NPN_ReleaseVariantValue(&ret_val);
}
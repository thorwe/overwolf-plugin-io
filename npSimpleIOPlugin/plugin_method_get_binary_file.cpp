#include "plugin_method_get_binary_file.h"

#include "utils/File.h"
#include "utils/Encoders.h"

#include <sstream>

// getBinaryFile( filename, size_limit, callback(status, data) )
PluginMethodGetBinaryFile::PluginMethodGetBinaryFile(NPObject* object, NPP npp) : 
  PluginMethod(object, npp) {
}

//virtual 
PluginMethod* PluginMethodGetBinaryFile::Clone(
  NPObject* object, 
  NPP npp, 
  const NPVariant *args, 
  uint32_t argCount, 
  NPVariant *result) {

  PluginMethodGetBinaryFile* clone = 
    new PluginMethodGetBinaryFile(object, npp);

  try {
    if (argCount < 3 ||
      !NPVARIANT_IS_STRING(args[0]) ||
      !NPVARIANT_IS_DOUBLE(args[1]) ||
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

    clone->limit_ = (int)NPVARIANT_TO_DOUBLE(args[1]);

    
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
bool PluginMethodGetBinaryFile::HasCallback() {
  return (nullptr != callback_);
}

// virtual
void PluginMethodGetBinaryFile::Execute() {
  std::wstring wide_filename = utils::Encoders::utf8_decode(filename_);

  status_ = utils::File::GetTextFile(wide_filename, output_, limit_);

  if (status_ != "success") {
    return;
  }

  std::ostringstream str;
  if (output_.size() > 0) {
    str << (int)output_[0];
  }
  for (size_t i = 1; i < output_.size(); ++i) {
    str << "," << (int)output_[i];
  }

  output_ = str.str();
}

// virtual
void PluginMethodGetBinaryFile::TriggerCallback() {
  NPVariant args[2];
  NPVariant ret_val;

  STRINGN_TO_NPVARIANT(
    output_.c_str(),
    output_.size(),
    args[1]);

  STRINGN_TO_NPVARIANT(
	  status_.c_str(),
	  status_.size(),
	  args[0]);

  // fire callback
  NPN_InvokeDefault(
    __super::npp_, 
    callback_, 
    args, 
    2, 
    &ret_val);

  NPN_ReleaseVariantValue(&ret_val);
}
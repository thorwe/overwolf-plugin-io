#include "plugin_method_get_file_times.h"

#include "utils/File.h"
#include "utils/Encoders.h"

#include <sstream>

// getFileTimes( filename, callback(status, creationTime, lastAccessTime, lastWriteTime ) )
PluginMethodGetFileTimes::PluginMethodGetFileTimes(NPObject* object, NPP npp) : 
PluginMethod(object, npp) {
}

//virtual 
PluginMethod* PluginMethodGetFileTimes::Clone(
  NPObject* object, 
  NPP npp, 
  const NPVariant *args, 
  uint32_t argCount, 
  NPVariant *result) {

    PluginMethodGetFileTimes* clone = 
      new PluginMethodGetFileTimes(object, npp);

    try {
      if (argCount < 3 ||
        !NPVARIANT_IS_STRING(args[0]) ||
        !NPVARIANT_IS_DOUBLE(args[1])) {
          NPN_SetException(
            __super::object_, 
            "invalid params passed to function");
          delete clone;
          return nullptr;
      }

      clone->callback_ = NPVARIANT_TO_OBJECT(args[1]);
      // add ref count to callback object so it won't delete
      NPN_RetainObject(clone->callback_);

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
bool PluginMethodGetFileTimes::HasCallback() {
  return (nullptr != callback_);
}

// virtual
void PluginMethodGetFileTimes::Execute() {
}

// virtual
void PluginMethodGetFileTimes::TriggerCallback() {
}
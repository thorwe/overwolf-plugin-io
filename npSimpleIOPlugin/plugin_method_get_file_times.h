/*
  Simple IO Plugin
  Copyright (c) 2014 Overwolf Ltd.
*/
#ifndef PLUGIN_METHODS_PLUGIN_METHOD_GET_FILE_TIMES_H_
#define PLUGIN_METHODS_PLUGIN_METHOD_GET_FILE_TIMES_H_

#include "plugin_method.h"
#include <string>

class PluginMethodGetFileTimes : public PluginMethod {
public:
  PluginMethodGetFileTimes(NPObject* object, NPP npp);

public:
  virtual PluginMethod* Clone(
    NPObject* object, 
    NPP npp, 
    const NPVariant *args, 
    uint32_t argCount, 
    NPVariant *result);
  virtual bool HasCallback();
  virtual void Execute();
  virtual void TriggerCallback();

protected:
  std::string filename_;
  NPObject* callback_;

  // callack
  bool status_;
  FILETIME creation_time_;
  FILETIME last_access_time_;
  FILETIME last_write_time_;
};


#endif // PLUGIN_METHODS_PLUGIN_METHOD_GET_FILE_TIMES_H_
/*
  Simple IO Plugin
  Copyright (c) 2014 Overwolf Ltd.
*/
#ifndef PLUGIN_METHODS_PLUGIN_METHOD_FILE_EXISTS_H_
#define PLUGIN_METHODS_PLUGIN_METHOD_FILE_EXISTS_H_

#include "plugin_method.h"
#include <string>

class PluginMethodFileExists : public PluginMethod {
public:
  PluginMethodFileExists(NPObject* object, NPP npp);

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
  bool exists_;
};


#endif // PLUGIN_METHODS_PLUGIN_METHOD_FILE_EXISTS_H_
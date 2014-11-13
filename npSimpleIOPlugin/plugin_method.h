/*
  Simple IO Plugin
  Copyright (c) 2014 Overwolf Ltd.
*/
#ifndef PLUGIN_METHODS_PLUGIN_METHOD_H_
#define PLUGIN_METHODS_PLUGIN_METHOD_H_

#include "nsScriptableObjectBase.h"

class PluginMethod {
public:
  PluginMethod(NPObject* object, NPP npp);
  virtual ~PluginMethod();

public:
  virtual PluginMethod* Clone(
    NPObject* object, 
    NPP npp, 
    const NPVariant *args, 
    uint32_t argCount, 
    NPVariant *result) = 0;
  virtual bool HasCallback() = 0;
  virtual void Execute() = 0;
  virtual void TriggerCallback() = 0;

protected:
  NPObject* object_;
  NPP npp_;
};

#endif // PLUGIN_METHODS_PLUGIN_METHOD_H_
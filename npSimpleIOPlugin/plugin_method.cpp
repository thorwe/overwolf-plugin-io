#include "plugin_method.h"

PluginMethod::PluginMethod(NPObject* object, NPP npp) : 
  object_(object),
  npp_(npp) {
}

PluginMethod::~PluginMethod() {

}

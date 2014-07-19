/*
  Simple IO Plugin
  Copyright (c) 2014 Overwolf Ltd.
*/
#include "nsPluginInstanceSimpleIO.h"
#include "nsScriptableObjectSimpleIO.h" // our specific API

// we use this to force our plugin container to shut down
// when no one is using it.  Browsers try to keep the plugin
// open for optimization reasons - we don't want it
int nsPluginInstanceSimpleIO::ref_count_ = 0;

////////////////////////////////////////
//
// nsPluginInstanceSimpleIO class implementation
//
nsPluginInstanceSimpleIO::nsPluginInstanceSimpleIO(NPP instance) :
  nsPluginInstanceBase(),
  instance_(instance),
  initialized_(FALSE),
  scriptable_object_(nullptr) {

  nsPluginInstanceSimpleIO::ref_count_++;
}

nsPluginInstanceSimpleIO::~nsPluginInstanceSimpleIO() {
  nsPluginInstanceSimpleIO::ref_count_--;

  if (0 == nsPluginInstanceSimpleIO::ref_count_) {
    PostQuitMessage(0);
  }
}

// NOTE:
// ------
// Overwolf plugins should not implement windows - NPAPI will
// probably be removed in the near feature and will be changed
// by a different method that will only support non-visual
// plugins
NPBool nsPluginInstanceSimpleIO::init(NPWindow* window) {
  // no GUI to init in windowless case
  initialized_ = TRUE;
  return TRUE;
}

void nsPluginInstanceSimpleIO::shut() {
  if (nullptr != scriptable_object_) {
    NPN_ReleaseObject(scriptable_object_);
  }

  initialized_ = FALSE;
}

NPBool nsPluginInstanceSimpleIO::isInitialized() {
  return initialized_;
}

// here we supply our scriptable object
NPError nsPluginInstanceSimpleIO::GetValue(
  NPPVariable variable, void* ret_value) {
  
  NPError rv = NPERR_INVALID_PARAM;

  switch (variable) {
    case NPPVpluginScriptableNPObject:
    {
      if (nullptr == scriptable_object_) {
        scriptable_object_ = 
          NPN_CreateObject(
            instance_, 
            GET_NPOBJECT_CLASS(nsScriptableObjectSimpleIO));

        NPN_RetainObject(scriptable_object_);

        ((nsScriptableObjectSimpleIO*)scriptable_object_)->Init();
        *(NPObject **)ret_value = scriptable_object_;
      }

      rv = NPERR_NO_ERROR;
      return rv;
    }
    default:
      break;
  }

  return rv;
}
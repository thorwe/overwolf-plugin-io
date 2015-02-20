#ifndef PLUGIN_METHODS_PLUGIN_METHOD_LIST_FILES_H_
#define PLUGIN_METHODS_PLUGIN_METHOD_LIST_FILES_H_

#include "plugin_method.h"
#include <string>

class PluginMethodListFiles : public PluginMethod {
public:
	PluginMethodListFiles(NPObject* object, NPP npp);

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
	std::string pathname_;
	NPObject* callback_;

	// callack
	bool status_;
	std::string output_;
};


#endif // PLUGIN_METHODS_PLUGIN_METHOD_LIST_FILES_H_
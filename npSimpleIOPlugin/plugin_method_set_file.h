#ifndef PLUGIN_METHODS_PLUGIN_METHOD_SET_FILE_H_
#define PLUGIN_METHODS_PLUGIN_METHOD_SET_FILE_H_

#include "plugin_method.h"
#include <string>

class PluginMethodSetFile : public PluginMethod
{
public:
	PluginMethodSetFile(NPObject* object, NPP npp);

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
	std::string content_;
	NPObject* callback_;

	// callback
	int status_;
};

#endif // PLUGIN_METHODS_PLUGIN_METHOD_SET_FILE_H_

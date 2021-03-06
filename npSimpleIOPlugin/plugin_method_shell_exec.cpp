#include "plugin_method_shell_exec.h"

#include "utils/File.h"
#include "utils/Encoders.h"

// shellExec( filename, callback(status) )
PluginMethodShellExec::PluginMethodShellExec(NPObject* object, NPP npp) :
PluginMethod(object, npp) {
}

//virtual 
PluginMethod* PluginMethodShellExec::Clone(
	NPObject* object,
	NPP npp,
	const NPVariant *args,
	uint32_t argCount,
	NPVariant *result) {

	PluginMethodShellExec* clone =
		new PluginMethodShellExec(object, npp);

	try {
		if (argCount < 2 ||
			!NPVARIANT_IS_STRING(args[0]) ||
			!NPVARIANT_IS_OBJECT(args[1])) {
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
	}
	catch (...) {

	}

	delete clone;
	return nullptr;
}

// virtual
bool PluginMethodShellExec::HasCallback() {
	return (nullptr != callback_);
}

// virtual
void PluginMethodShellExec::Execute() {
	std::wstring wide_filename = utils::Encoders::utf8_decode(filename_);

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	HINSTANCE hInstance = ShellExecuteW(GetDesktopWindow(), L"open", wide_filename.c_str(), NULL, NULL, SW_SHOW);
	status_ = (int)hInstance;
}

// virtual
void PluginMethodShellExec::TriggerCallback() {
	NPVariant arg;
	NPVariant ret_val;

	INT32_TO_NPVARIANT(
		status_,
		arg);

	// fire callback
	NPN_InvokeDefault(
		__super::npp_,
		callback_,
		&arg,
		1,
		&ret_val);

	NPN_ReleaseVariantValue(&ret_val);
}
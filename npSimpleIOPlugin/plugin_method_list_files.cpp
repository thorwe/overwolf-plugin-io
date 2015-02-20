#include "plugin_method_list_files.h"

#include "utils/File.h"
#include "utils/Encoders.h"

// listfiles(pathName, callback(status, data))
PluginMethodListFiles::PluginMethodListFiles(NPObject* object, NPP npp) :
PluginMethod(object, npp) {
}

//virtual 
PluginMethod* PluginMethodListFiles::Clone(
	NPObject* object,
	NPP npp,
	const NPVariant *args,
	uint32_t argCount,
	NPVariant *result) {

	PluginMethodListFiles* clone =
		new PluginMethodListFiles(object, npp);

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

		// convert into std::string
		clone->pathname_.append(
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
bool PluginMethodListFiles::HasCallback() {
	return (nullptr != callback_);
}

// virtual
void PluginMethodListFiles::Execute() {
	std::wstring wide_pathname = utils::Encoders::utf8_decode(pathname_);

	status_ = utils::File::ListDirectoryContents(wide_pathname, output_);
}

// virtual
void PluginMethodListFiles::TriggerCallback() {
	NPVariant args[2];
	NPVariant ret_val;

	BOOLEAN_TO_NPVARIANT(
		status_,
		args[0]);

	STRINGN_TO_NPVARIANT(
		output_.c_str(),
		output_.size(),
		args[1]);

	// fire callback
	NPN_InvokeDefault(
		__super::npp_,
		callback_,
		args,
		2,
		&ret_val);

	NPN_ReleaseVariantValue(&ret_val);
}
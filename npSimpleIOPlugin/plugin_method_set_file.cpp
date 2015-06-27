#include "plugin_method_set_file.h"

#include "utils/File.h"
#include "utils/Encoders.h"

// setFile( filename, content, callback(status) )
PluginMethodSetFile::PluginMethodSetFile(NPObject* object, NPP npp) :
PluginMethod(object, npp) {
}

//virtual 
PluginMethod* PluginMethodSetFile::Clone(
	NPObject* object,
	NPP npp,
	const NPVariant *args,
	uint32_t argCount,
	NPVariant *result) {

	PluginMethodSetFile* clone =
		new PluginMethodSetFile(object, npp);

	try {
		if (argCount != 3 ||
			!NPVARIANT_IS_STRING(args[0]) ||
			!NPVARIANT_IS_STRING(args[1]) ||
			!NPVARIANT_IS_OBJECT(args[2])) {
			NPN_SetException(
				__super::object_,
				"invalid params passed to function");
			delete clone;
			return nullptr;
		}

		clone->callback_ = NPVARIANT_TO_OBJECT(args[2]);
		// add ref count to callback object so it won't delete
		NPN_RetainObject(clone->callback_);

		clone->filename_.append(
			NPVARIANT_TO_STRING(args[0]).UTF8Characters,
			NPVARIANT_TO_STRING(args[0]).UTF8Length);

		clone->content_.append(
			NPVARIANT_TO_STRING(args[1]).UTF8Characters,
			NPVARIANT_TO_STRING(args[1]).UTF8Length);

		return clone;
	}
	catch (...) {

	}

	delete clone;
	return nullptr;
}

// virtual
bool PluginMethodSetFile::HasCallback() {
	return (nullptr != callback_);
}

// virtual
void PluginMethodSetFile::Execute() {
	std::wstring wide_filename = utils::Encoders::utf8_decode(filename_);
	status_ = utils::File::SetFile(wide_filename, content_);
}

// virtual
void PluginMethodSetFile::TriggerCallback() {
	NPVariant arg;
	NPVariant ret_val;

	STRINGN_TO_NPVARIANT(
		status_.c_str(),
		status_.size(),
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

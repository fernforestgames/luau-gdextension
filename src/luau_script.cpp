#include "luau_script.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

// ============================================================================
// LuauScript
// ============================================================================

LuauScript::LuauScript()
{
}

LuauScript::~LuauScript()
{
}

void LuauScript::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("set_source_code", "source"), &LuauScript::set_source_code);
	ClassDB::bind_method(D_METHOD("get_source_code"), &LuauScript::get_source_code);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "source_code", PROPERTY_HINT_MULTILINE_TEXT, "", PROPERTY_USAGE_NO_EDITOR), "set_source_code", "get_source_code");
}

void LuauScript::set_source_code(const String &p_source)
{
	source_code = p_source;
}

String LuauScript::get_source_code() const
{
	return source_code;
}

Error LuauScript::load_source_code(const String &p_path)
{
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ);

	if (file.is_null())
	{
		Error err = FileAccess::get_open_error();
		ERR_PRINT(vformat("Cannot open Luau script file '%s'.", p_path));
		return err;
	}

	String source = file->get_as_text();
	file->close();

	set_source_code(source);
	set_path_cache(p_path);

	return OK;
}

// ============================================================================
// ResourceFormatLoaderLuauScript
// ============================================================================

Variant ResourceFormatLoaderLuauScript::_load(const String &p_path, const String &p_original_path, bool p_use_sub_threads, int32_t p_cache_mode) const
{
	Ref<LuauScript> script;
	script.instantiate();

	Error err = script->load_source_code(p_original_path);
	if (err != OK)
	{
		ERR_PRINT(vformat("Failed to load Luau script '%s' with error %d.", p_original_path, err));
		return Variant();
	}

	return script;
}

PackedStringArray ResourceFormatLoaderLuauScript::_get_recognized_extensions() const
{
	PackedStringArray extensions;
	extensions.push_back("lua");
	extensions.push_back("luau");
	return extensions;
}

bool ResourceFormatLoaderLuauScript::_handles_type(const StringName &p_type) const
{
	return p_type == StringName("Resource") || p_type == StringName("LuauScript");
}

String ResourceFormatLoaderLuauScript::_get_resource_type(const String &p_path) const
{
	String extension = p_path.get_extension().to_lower();
	if (extension == "lua" || extension == "luau")
	{
		return "LuauScript";
	}
	return "";
}

// ============================================================================
// ResourceFormatSaverLuauScript
// ============================================================================

Error ResourceFormatSaverLuauScript::_save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags)
{
	Ref<LuauScript> script = p_resource;
	if (script.is_null())
	{
		return ERR_INVALID_PARAMETER;
	}

	String source = script->get_source_code();

	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::WRITE);

	if (file.is_null())
	{
		Error err = FileAccess::get_open_error();
		ERR_PRINT(vformat("Cannot save Luau script file '%s'.", p_path));
		return err;
	}

	file->store_string(source);
	file->close();

	if (file->get_error() != OK && file->get_error() != ERR_FILE_EOF)
	{
		return ERR_CANT_CREATE;
	}

	return OK;
}

PackedStringArray ResourceFormatSaverLuauScript::_get_recognized_extensions(const Ref<Resource> &p_resource) const
{
	PackedStringArray extensions;

	Ref<LuauScript> script = p_resource;
	if (script.is_valid())
	{
		extensions.push_back("lua");
		extensions.push_back("luau");
	}

	return extensions;
}

bool ResourceFormatSaverLuauScript::_recognize(const Ref<Resource> &p_resource) const
{
	return Object::cast_to<LuauScript>(p_resource.ptr()) != nullptr;
}

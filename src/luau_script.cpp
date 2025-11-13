#include "luau_script.h"

#include "luau.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace gdluau;
using namespace godot;

void LuauScript::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("set_source_code", "source"), &LuauScript::set_source_code);
	ClassDB::bind_method(D_METHOD("get_source_code"), &LuauScript::get_source_code);

	ClassDB::bind_method(D_METHOD("set_compile_options", "options"), &LuauScript::set_compile_options);
	ClassDB::bind_method(D_METHOD("get_compile_options"), &LuauScript::get_compile_options);

	ClassDB::bind_method(D_METHOD("compile", "force_recompile"), &LuauScript::compile, DEFVAL(false));

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "source_code", PROPERTY_HINT_MULTILINE_TEXT), "set_source_code", "get_source_code");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "compile_options", PROPERTY_HINT_RESOURCE_TYPE, "LuaCompileOptions", PROPERTY_USAGE_NONE), "set_compile_options", "get_compile_options");
}

void LuauScript::_reset_state()
{
	source_code = String();
	compile_options.unref();
	cached_bytecode.clear();
}

void LuauScript::set_source_code(const String &p_source)
{
	if (p_source == source_code)
	{
		return;
	}

	cached_bytecode.clear();
	source_code = p_source;
}

const String &LuauScript::get_source_code() const
{
	return source_code;
}

void LuauScript::set_compile_options(LuaCompileOptions *p_options)
{
	if (p_options == compile_options.ptr())
	{
		return;
	}

	cached_bytecode.clear();
	compile_options.reference_ptr(p_options);
}

LuaCompileOptions *LuauScript::get_compile_options() const
{
	return compile_options.ptr();
}

const PackedByteArray &LuauScript::compile(bool p_force_recompile)
{
	if (cached_bytecode.is_empty() || p_force_recompile)
	{
		cached_bytecode = Luau::compile(source_code, compile_options.ptr());
	}

	return cached_bytecode;
}

Variant ResourceFormatLoaderLuauScript::_load(const String &p_path, const String &p_original_path, bool p_use_sub_threads, int32_t p_cache_mode) const
{
	Ref<LuauScript> script;

	if (p_cache_mode != CACHE_MODE_IGNORE && p_cache_mode != CACHE_MODE_IGNORE_DEEP && ResourceLoader::get_singleton()->has_cached(p_path))
	{
		script = ResourceLoader::get_singleton()->get_cached_ref(p_path);
	}

	if (script.is_null())
	{
		script.instantiate();
	}

	if (p_cache_mode == CACHE_MODE_REPLACE || p_cache_mode == CACHE_MODE_REPLACE_DEEP || script->get_source_code().is_empty())
	{
		// Load from disk
		Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ);
		ERR_FAIL_COND_V_MSG(file.is_null(), FileAccess::get_open_error(), vformat("Cannot open Luau script file '%s'.", p_path));

		String source = file->get_as_text();
		file->close();

		script->set_source_code(source);

		if (p_cache_mode == CACHE_MODE_IGNORE || p_cache_mode == CACHE_MODE_IGNORE_DEEP)
		{
			// Set the path of the resource but do not cache it
			script->set_path_cache(p_path);
		}
		else
		{
			// After loading (in CACHE_MODE_REUSE) or reloading (CACHE_MODE_REPLACE*), cache it
			script->take_over_path(p_path);
		}
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

Error ResourceFormatSaverLuauScript::_save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags)
{
	Ref<LuauScript> script = p_resource;
	ERR_FAIL_COND_V_MSG(script.is_null(), ERR_INVALID_PARAMETER, "Invalid LuauScript resource.");

	// TODO: Support persisting `compile_options` (maybe this needs a custom importer)

	String source = script->get_source_code();
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::WRITE);
	ERR_FAIL_COND_V_MSG(file.is_null(), FileAccess::get_open_error(), vformat("Cannot save Luau script file '%s'.", p_path));

	file->store_string(source);
	file->close();
	return file->get_error();
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

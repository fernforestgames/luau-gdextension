#pragma once

#include "lua_compileoptions.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/resource_format_loader.hpp>
#include <godot_cpp/classes/resource_format_saver.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace gdluau
{
	using namespace godot;

	class LuauScript : public Resource
	{
		GDCLASS(LuauScript, Resource)

	private:
		String source_code;
		Ref<LuaCompileOptions> compile_options;
		PackedByteArray cached_bytecode;

	protected:
		static void _bind_methods();

	public:
		LuauScript() {}

		virtual void _reset_state() override;

		void set_source_code(const String &p_source);
		const String &get_source_code() const;

		void set_compile_options(LuaCompileOptions *p_options);
		LuaCompileOptions *get_compile_options() const;

		const PackedByteArray &compile(bool p_force_recompile = false);
	};

	class ResourceFormatLoaderLuauScript : public ResourceFormatLoader
	{
		GDCLASS(ResourceFormatLoaderLuauScript, ResourceFormatLoader)

	protected:
		static void _bind_methods() {}

	public:
		virtual Variant _load(const String &p_path, const String &p_original_path, bool p_use_sub_threads, int32_t p_cache_mode) const override;
		virtual PackedStringArray _get_recognized_extensions() const override;
		virtual bool _handles_type(const StringName &p_type) const override;
		virtual String _get_resource_type(const String &p_path) const override;
	};

	class ResourceFormatSaverLuauScript : public ResourceFormatSaver
	{
		GDCLASS(ResourceFormatSaverLuauScript, ResourceFormatSaver)

	protected:
		static void _bind_methods() {}

	public:
		virtual Error _save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags) override;
		virtual PackedStringArray _get_recognized_extensions(const Ref<Resource> &p_resource) const override;
		virtual bool _recognize(const Ref<Resource> &p_resource) const override;
	};
} // namespace gdluau

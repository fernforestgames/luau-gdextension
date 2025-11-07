#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/resource_format_loader.hpp>
#include <godot_cpp/classes/resource_format_saver.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace gdluau
{
	using namespace godot;

	/// A resource representing a Luau script file (.lua or .luau).
	/// This class stores the source code and provides integration with Godot's resource system.
	class LuauScript : public Resource
	{
		GDCLASS(LuauScript, Resource)

	private:
		String source_code;

	protected:
		static void _bind_methods();

	public:
		LuauScript();
		~LuauScript();

		// Source code management
		void set_source_code(const String &p_source);
		String get_source_code() const;

		// Resource loading
		Error load_source_code(const String &p_path);
	};

	/// ResourceFormatLoader for Luau scripts (.lua, .luau files)
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

	/// ResourceFormatSaver for Luau scripts (.lua, .luau files)
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

extends RefCounted

const lua_userdata_tag: int = 42

func _init() -> void:
	print("Class name: ", get_class())
	print("Has lua_userdata_tag: ", "lua_userdata_tag" in self)
	if "lua_userdata_tag" in self:
		print("lua_userdata_tag value: ", get("lua_userdata_tag"))

	# Also try with get_script()
	var script = get_script()
	if script:
		print("Script class: ", script.get_class())
		var constant_map = script.get_script_constant_map()
		print("Constants: ", constant_map)

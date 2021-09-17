
use_git_submodule("${vkcv_asset_loader_lib_path}/json" json_status)

if (${json_status})
	set(JSON_BuildTests OFF CACHE INTERNAL "")
	
	add_subdirectory(${vkcv_asset_loader_lib}/json)
	
	list(APPEND vkcv_asset_loader_libraries nlohmann_json::nlohmann_json)
endif ()

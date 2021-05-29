
if (EXISTS "${vkcv_asset_loader_lib_path}/json")
	set(JSON_BuildTests OFF CACHE INTERNAL "")
	
	add_subdirectory(${vkcv_asset_loader_lib}/json)
	
	list(APPEND vkcv_asset_loader_libraries nlohmann_json::nlohmann_json)
else()
	message(WARNING "NLOHMANN_JSON is required..! Update the submodules!")
endif ()

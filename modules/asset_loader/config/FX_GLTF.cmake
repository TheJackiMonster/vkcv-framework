
use_git_submodule("${vkcv_asset_loader_lib_path}/fx-gltf" fx_gltf_status)

if (${fx_gltf_status})
	set(FX_GLTF_INSTALL OFF CACHE INTERNAL "")
	set(FX_GLTF_USE_INSTALLED_DEPS OFF CACHE INTERNAL "")
	set(BUILD_TESTING OFF CACHE INTERNAL "")
	
	add_subdirectory(${vkcv_asset_loader_lib}/fx-gltf)
	
	list(APPEND vkcv_asset_loader_libraries fx-gltf)
endif ()

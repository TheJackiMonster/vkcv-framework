
if (EXISTS "${vkcv_asset_loader_lib_path}/fx-gltf")
	set(FX_GLTF_INSTALL OFF CACHE INTERNAL "")
	set(FX_GLTF_USE_INSTALLED_DEPS OFF CACHE INTERNAL "")
	set(BUILD_TESTING OFF CACHE INTERNAL "")
	
	add_subdirectory(${vkcv_asset_loader_lib}/fx-gltf)
	
	list(APPEND vkcv_asset_loader_libraries fx-gltf)
else()
	message(WARNING "FX-GLTF is required..! Update the submodules!")
endif ()

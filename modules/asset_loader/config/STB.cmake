
if (EXISTS "${vkcv_asset_loader_lib_path}/stb")
	list(APPEND vkcv_asset_loader_includes ${vkcv_asset_loader_lib}/stb)
	
	list(APPEND vkcv_asset_loader_definitions STB_IMAGE_IMPLEMENTATION)
	list(APPEND vkcv_asset_loader_definitions STBI_ONLY_JPEG)
	list(APPEND vkcv_asset_loader_definitions STBI_ONLY_PNG)
else()
	message(WARNING "STB is required..! Update the submodules!")
endif ()

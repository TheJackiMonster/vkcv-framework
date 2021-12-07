
use_git_submodule("${vkcv_asset_loader_lib_path}/stb" stb_status)

if (${stb_status})
	list(APPEND vkcv_asset_loader_includes ${vkcv_asset_loader_lib}/stb)
	
	list(APPEND vkcv_asset_loader_definitions STB_IMAGE_IMPLEMENTATION)
	list(APPEND vkcv_asset_loader_definitions STBI_ONLY_JPEG)
	list(APPEND vkcv_asset_loader_definitions STBI_ONLY_PNG)
endif ()

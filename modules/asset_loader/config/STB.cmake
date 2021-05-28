
if (EXISTS "${vkcv_asset_loader_lib_path}/stb")
	list(APPEND vkcv_asset_loader_includes ${vkcv_asset_loader_lib}/stb)
else()
	message(WARNING "STB is required..! Update the submodules!")
endif ()

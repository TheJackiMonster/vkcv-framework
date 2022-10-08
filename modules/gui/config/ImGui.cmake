
use_git_submodule("${vkcv_gui_lib_path}/imgui" imgui_status)

if (${imgui_status})
	list(APPEND vkcv_imgui_sources ${vkcv_gui_lib_path}/imgui/backends/imgui_impl_glfw.cpp)
	list(APPEND vkcv_imgui_sources ${vkcv_gui_lib_path}/imgui/backends/imgui_impl_glfw.h)
	list(APPEND vkcv_imgui_sources ${vkcv_gui_lib_path}/imgui/backends/imgui_impl_vulkan.cpp)
	list(APPEND vkcv_imgui_sources ${vkcv_gui_lib_path}/imgui/backends/imgui_impl_vulkan.h)
	list(APPEND vkcv_imgui_sources ${vkcv_gui_lib_path}/imgui/imconfig.h)
	list(APPEND vkcv_imgui_sources ${vkcv_gui_lib_path}/imgui/imgui.cpp)
	list(APPEND vkcv_imgui_sources ${vkcv_gui_lib_path}/imgui/imgui.h)
	list(APPEND vkcv_imgui_sources ${vkcv_gui_lib_path}/imgui/imgui_draw.cpp)
	list(APPEND vkcv_imgui_sources ${vkcv_gui_lib_path}/imgui/imgui_demo.cpp)
	list(APPEND vkcv_imgui_sources ${vkcv_gui_lib_path}/imgui/imgui_internal.h)
	list(APPEND vkcv_imgui_sources ${vkcv_gui_lib_path}/imgui/imgui_tables.cpp)
	list(APPEND vkcv_imgui_sources ${vkcv_gui_lib_path}/imgui/imgui_widgets.cpp)
	list(APPEND vkcv_imgui_sources ${vkcv_gui_lib_path}/imgui/imstb_rectpack.h)
	list(APPEND vkcv_imgui_sources ${vkcv_gui_lib_path}/imgui/imstb_textedit.h)
	list(APPEND vkcv_imgui_sources ${vkcv_gui_lib_path}/imgui/imstb_truetype.h)
	
	list(APPEND vkcv_imgui_includes ${vkcv_gui_lib}/imgui)
	list(APPEND vkcv_imgui_includes ${vkcv_gui_lib}/imgui/backend)
	
	list(APPEND vkcv_gui_include ${vkcv_gui_lib})
	
	list(APPEND vkcv_gui_defines IMGUI_DISABLE_WIN32_FUNCTIONS=1)
endif ()

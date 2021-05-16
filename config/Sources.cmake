
# adding all source files and header files of the framework:
set(vkcv_sources
		${vkcv_include}/vkcv/Context.hpp
		${vkcv_source}/vkcv/Context.cpp

		${vkcv_include}/vkcv/Core.hpp
		${vkcv_source}/vkcv/Core.cpp

		${vkcv_include}/vkcv/PassConfig.hpp
		${vkcv_source}/vkcv/PassConfig.cpp

		${vkcv_source}/vkcv/PassManager.hpp
		${vkcv_source}/vkcv/PassManager.cpp

		${vkcv_include}/vkcv/Handles.hpp
		${vkcv_source}/vkcv/Handles.cpp

		${vkcv_include}/vkcv/Window.hpp
		${vkcv_source}/vkcv/Window.cpp

		${vkcv_include}/vkcv/SwapChain.hpp
		${vkcv_source}/vkcv/SwapChain.cpp

		${vkcv_include}/vkcv/ShaderProgram.hpp
		${vkcv_source}/vkcv/ShaderProgram.cpp

		${vkcv_include}/vkcv/Pipeline.hpp
		${vkcv_source}/vkcv/Pipeline.cpp
)

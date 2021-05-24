
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

		${vkcv_include}/vkcv/Buffer.hpp
		
		${vkcv_include}/vkcv/BufferManager.hpp
		${vkcv_source}/vkcv/BufferManager.cpp

		${vkcv_include}/vkcv/SwapChain.hpp
		${vkcv_source}/vkcv/SwapChain.cpp

		${vkcv_include}/vkcv/ShaderProgram.hpp
		${vkcv_source}/vkcv/ShaderProgram.cpp

		${vkcv_include}/vkcv/PipelineConfig.hpp
		${vkcv_source}/vkcv/PipelineConfig.cpp

		${vkcv_source}/vkcv/PipelineManager.hpp
		${vkcv_source}/vkcv/PipelineManager.cpp
        
        ${vkcv_include}/vkcv/CommandResources.hpp
        ${vkcv_source}/vkcv/CommandResources.cpp
        
        ${vkcv_include}/vkcv/SyncResources.hpp
        ${vkcv_source}/vkcv/SyncResources.cpp
        
        ${vkcv_include}/vkcv/QueueManager.hpp
        ${vkcv_source}/vkcv/QueueManager.cpp
        
        ${vkcv_source}/vkcv/Surface.hpp
        ${vkcv_source}/vkcv/Surface.cpp
        
        ${vkcv_source}/vkcv/ImageLayoutTransitions.hpp
        ${vkcv_source}/vkcv/ImageLayoutTransitions.cpp
        
        ${vkcv_source}/vkcv/Framebuffer.hpp
        ${vkcv_source}/vkcv/Framebuffer.cpp
)

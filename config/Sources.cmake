
# adding all source files and header files of the framework:
set(vkcv_sources
		${vkcv_include}/vkcv/Context.hpp
		${vkcv_source}/vkcv/Context.cpp

		${vkcv_include}/vkcv/Core.hpp
		${vkcv_source}/vkcv/Core.cpp
		
		${vkcv_include}/vkcv/File.hpp
		${vkcv_source}/vkcv/File.cpp

		${vkcv_include}/vkcv/PassConfig.hpp
		${vkcv_source}/vkcv/PassConfig.cpp

		${vkcv_source}/vkcv/PassManager.hpp
		${vkcv_source}/vkcv/PassManager.cpp

		${vkcv_include}/vkcv/Handles.hpp
		${vkcv_source}/vkcv/Handles.cpp

		${vkcv_include}/vkcv/Window.hpp
		${vkcv_source}/vkcv/Window.cpp

		${vkcv_include}/vkcv/Buffer.hpp
		
		${vkcv_include}/vkcv/PushConstants.hpp
		
		${vkcv_include}/vkcv/BufferManager.hpp
		${vkcv_source}/vkcv/BufferManager.cpp

		${vkcv_include}/vkcv/Image.hpp
		${vkcv_source}/vkcv/Image.cpp

		${vkcv_source}/vkcv/ImageManager.hpp
		${vkcv_source}/vkcv/ImageManager.cpp
		
		${vkcv_include}/vkcv/Logger.hpp

		${vkcv_include}/vkcv/Swapchain.hpp
		${vkcv_source}/vkcv/Swapchain.cpp
		
		${vkcv_include}/vkcv/ShaderStage.hpp
		
		${vkcv_include}/vkcv/ShaderProgram.hpp
		${vkcv_source}/vkcv/ShaderProgram.cpp

		${vkcv_include}/vkcv/PipelineConfig.hpp

		${vkcv_source}/vkcv/PipelineManager.hpp
		${vkcv_source}/vkcv/PipelineManager.cpp
        
        ${vkcv_include}/vkcv/CommandResources.hpp
        ${vkcv_source}/vkcv/CommandResources.cpp
        
        ${vkcv_include}/vkcv/SyncResources.hpp
        ${vkcv_source}/vkcv/SyncResources.cpp
        
        ${vkcv_include}/vkcv/QueueManager.hpp
        ${vkcv_source}/vkcv/QueueManager.cpp

        ${vkcv_source}/vkcv/ImageLayoutTransitions.hpp
        ${vkcv_source}/vkcv/ImageLayoutTransitions.cpp

		${vkcv_include}/vkcv/VertexLayout.hpp
		${vkcv_source}/vkcv/VertexLayout.cpp

		${vkcv_include}/vkcv/Event.hpp

		${vkcv_source}/vkcv/DescriptorManager.hpp
		${vkcv_source}/vkcv/DescriptorManager.cpp

		${vkcv_include}/vkcv/DescriptorConfig.hpp
		${vkcv_source}/vkcv/DescriptorConfig.cpp
		
		${vkcv_source}/vkcv/SamplerManager.hpp
		${vkcv_source}/vkcv/SamplerManager.cpp
        
        ${vkcv_include}/vkcv/DescriptorWrites.hpp
        
        ${vkcv_include}/vkcv/DrawcallRecording.hpp
        ${vkcv_source}/vkcv/DrawcallRecording.cpp
        
        ${vkcv_include}/vkcv/CommandStreamManager.hpp
        ${vkcv_source}/vkcv/CommandStreamManager.cpp
        
        ${vkcv_include}/vkcv/CommandRecordingFunctionTypes.hpp
        
        ${vkcv_include}/vkcv/ImageConfig.hpp
        ${vkcv_source}/vkcv/ImageConfig.cpp
)

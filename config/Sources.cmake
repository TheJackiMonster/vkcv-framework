
# adding all source files and header files of the framework:
set(vkcv_sources
		${vkcv_include}/vkcv/Features.hpp
		${vkcv_source}/vkcv/Features.cpp
		
		${vkcv_include}/vkcv/FeatureManager.hpp
		${vkcv_source}/vkcv/FeatureManager.cpp
		
		${vkcv_include}/vkcv/Context.hpp
		${vkcv_source}/vkcv/Context.cpp

		${vkcv_include}/vkcv/Core.hpp
		${vkcv_source}/vkcv/Core.cpp
		
		${vkcv_include}/vkcv/File.hpp
		${vkcv_source}/vkcv/File.cpp

		${vkcv_include}/vkcv/PassConfig.hpp

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
		
		${vkcv_include}/vkcv/Surface.hpp
		${vkcv_source}/vkcv/Surface.cpp

		${vkcv_include}/vkcv/Swapchain.hpp
		${vkcv_source}/vkcv/Swapchain.cpp
		
		${vkcv_include}/vkcv/ShaderStage.hpp
		
		${vkcv_include}/vkcv/ShaderProgram.hpp
		${vkcv_source}/vkcv/ShaderProgram.cpp

		${vkcv_include}/vkcv/GraphicsPipelineConfig.hpp
		${vkcv_include}/vkcv/ComputePipelineConfig.hpp

		${vkcv_source}/vkcv/ComputePipelineManager.hpp
		${vkcv_source}/vkcv/ComputePipelineManager.cpp

		${vkcv_source}/vkcv/GraphicsPipelineManager.hpp
		${vkcv_source}/vkcv/GraphicsPipelineManager.cpp

        ${vkcv_include}/vkcv/CommandResources.hpp
        ${vkcv_source}/vkcv/CommandResources.cpp
        
        ${vkcv_include}/vkcv/SyncResources.hpp
        ${vkcv_source}/vkcv/SyncResources.cpp
        
        ${vkcv_include}/vkcv/QueueManager.hpp
        ${vkcv_source}/vkcv/QueueManager.cpp

		${vkcv_include}/vkcv/VertexLayout.hpp
		${vkcv_source}/vkcv/VertexLayout.cpp

		${vkcv_include}/vkcv/Event.hpp

		${vkcv_source}/vkcv/DescriptorManager.hpp
		${vkcv_source}/vkcv/DescriptorManager.cpp

		${vkcv_include}/vkcv/DescriptorConfig.hpp
		${vkcv_source}/vkcv/DescriptorConfig.cpp
		
		${vkcv_include}/vkcv/DescriptorWrites.hpp
		${vkcv_source}/vkcv/DescriptorWrites.cpp
		
		${vkcv_source}/vkcv/SamplerManager.hpp
		${vkcv_source}/vkcv/SamplerManager.cpp

		${vkcv_source}/vkcv/WindowManager.hpp
		${vkcv_source}/vkcv/WindowManager.cpp

		${vkcv_source}/vkcv/SwapchainManager.hpp
		${vkcv_source}/vkcv/SwapchainManager.cpp
        
        ${vkcv_include}/vkcv/DescriptorWrites.hpp
        
        ${vkcv_include}/vkcv/DrawcallRecording.hpp
        ${vkcv_source}/vkcv/DrawcallRecording.cpp
        
        ${vkcv_source}/vkcv/CommandStreamManager.hpp
        ${vkcv_source}/vkcv/CommandStreamManager.cpp
        
        ${vkcv_include}/vkcv/CommandRecordingFunctionTypes.hpp
        
        ${vkcv_include}/vkcv/ImageConfig.hpp
        ${vkcv_source}/vkcv/ImageConfig.cpp
		
		${vkcv_include}/vkcv/Downsampler.hpp
		${vkcv_source}/vkcv/Downsampler.cpp
		
		${vkcv_include}/vkcv/BlitDownsampler.hpp
		${vkcv_source}/vkcv/BlitDownsampler.cpp
		
		${vkcv_include}/vkcv/Sampler.hpp
		
		${vkcv_include}/vkcv/Result.hpp
)

if (BUILD_CLANG_FORMAT)
	message(STATUS "Clang-Format: ON")
	
	# add clang-format as target if installed
	include(${vkcv_config}/ext/ClangFormat.cmake)
else()
	message(STATUS "Clang-Format: OFF")
endif()

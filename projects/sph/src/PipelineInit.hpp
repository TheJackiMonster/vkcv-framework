#pragma once
#include <vkcv/Core.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include <fstream>

class PipelineInit{
public:
    /**
     * Helperfunction to initialize a compute Pipeline. Goal is to reduce repetitive code in main.
     * @param pCore Pointer to core object
     * @param shaderStage Type of shaderstage - currently only supports COMPUTE
     * @param includePath filepath to shaderprogram
     * @param pipeline handle of the pipeline that is to be initialized. This handle is replaced with the handle of the generated pipeline
     * @return returns the descriptorset handle from the generated descriptorset of the reflected shader
     */
    static vkcv::DescriptorSetHandle ComputePipelineInit(vkcv::Core *pCore,
                                                         vkcv::ShaderStage shaderStage,
                                                         std::filesystem::path includePath,
                                                         vkcv::ComputePipelineHandle& pipeline);
};

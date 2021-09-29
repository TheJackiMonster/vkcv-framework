#pragma once
#include <vkcv/Core.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include <fstream>

class PipelineInit{
public:
    static vkcv::DescriptorSetHandle ComputePipelineInit(vkcv::Core *pCore,
                                                         vkcv::ShaderStage shaderStage,
                                                         std::filesystem::path includePath,
                                                         vkcv::ComputePipelineHandle& pipeline);
};

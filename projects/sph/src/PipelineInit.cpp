#include "PipelineInit.hpp"

vkcv::DescriptorSetHandle PipelineInit::ComputePipelineInit(vkcv::Core *pCore, vkcv::ShaderStage shaderStage, std::filesystem::path includePath,
                                vkcv::ComputePipelineHandle &pipeline) {
    vkcv::ShaderProgram shaderProgram;
    vkcv::shader::GLSLCompiler compiler;
    compiler.compile(shaderStage, includePath,
                     [&](vkcv::ShaderStage shaderStage1, const std::filesystem::path& path1) {shaderProgram.addShader(shaderStage1, path1);
    });
    vkcv::DescriptorSetLayoutHandle descriptorSetLayout = pCore->createDescriptorSetLayout(
            shaderProgram.getReflectedDescriptors().at(0));
    vkcv::DescriptorSetHandle descriptorSet = pCore->createDescriptorSet(descriptorSetLayout);

    const std::vector<vkcv::VertexAttachment> vertexAttachments = shaderProgram.getVertexAttachments();

    std::vector<vkcv::VertexBinding> bindings;
    for (size_t i = 0; i < vertexAttachments.size(); i++) {
        bindings.push_back(vkcv::VertexBinding(i, { vertexAttachments[i] }));
    }
    const vkcv::VertexLayout layout(bindings);

    pipeline = pCore->createComputePipeline({
            shaderProgram,
            { descriptorSetLayout }
	});

    return  descriptorSet;
}
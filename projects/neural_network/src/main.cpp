#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <chrono>
#include <vkcv/shader/GLSLCompiler.hpp>

int main(int argc, const char** argv) {

    const char* applicationName = "Neural-Network";

    uint32_t windowWidth = 800;
    uint32_t windowHeight = 600;
    vkcv::Window window = vkcv::Window::create(
        applicationName,
        windowWidth,
        windowHeight,
        true
    );

    vkcv::Core core = vkcv::Core::create(
        window,
        applicationName,
        VK_MAKE_VERSION(0, 0, 1),
        { vk::QueueFlagBits::eTransfer, vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute },
        { "VK_KHR_swapchain" }
    );

    int input[64] = { 0 };
    std::fill_n(input, 64, 2);
    vkcv::Buffer<int> inputBuffer = core.createBuffer<int>(vkcv::BufferType::STORAGE, 64, vkcv::BufferMemoryType::HOST_VISIBLE);
    inputBuffer.fill(input);

    vkcv::PassConfig computePassDefinition({});
    vkcv::PassHandle computePass = core.createPass(computePassDefinition);
    
    if (!computePass)
    {
        std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
        return EXIT_FAILURE;
    }

    // shader path
    std::string shaderPathCompute = "resources/shaders/simpleShader.comp";

    vkcv::shader::GLSLCompiler compiler;
    vkcv::ShaderProgram computeShaderProgram{};
    compiler.compile(vkcv::ShaderStage::COMPUTE, shaderPathCompute, [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
        computeShaderProgram.addShader(shaderStage, path);
    });


    vkcv::DescriptorSetHandle computeDescriptorSet = core.createDescriptorSet(computeShaderProgram.getReflectedDescriptors()[0]);

    vkcv::PipelineHandle computePipeline = core.createComputePipeline(computeShaderProgram, { core.getDescriptorSet(computeDescriptorSet).layout });

    vkcv::DescriptorWrites computeWrites;
    computeWrites.storageBufferWrites = { vkcv::BufferDescriptorWrite(0,inputBuffer.getHandle()) };
    core.writeDescriptorSet(computeDescriptorSet,  computeWrites);

    if (!computePipeline)
    {
        std::cout << "Error. Could not create compute pipeline. Exiting." << std::endl;
        return EXIT_FAILURE;
    }
    for(int i =0;i<3;i++){
    auto cmdStream = core.createCommandStream(vkcv::QueueType::Compute);
    uint32_t computeDispatchCount[3] = {64,1,1 };

    vkcv::PushConstants pushConstantsCompute(sizeof(int));
    pushConstantsCompute.appendDrawcall(1);

    core.recordComputeDispatchToCmdStream(cmdStream,
        computePipeline,
        computeDispatchCount,
        { vkcv::DescriptorSetUsage(0,core.getDescriptorSet(computeDescriptorSet).vulkanHandle) },
        pushConstantsCompute);

    int output[64] = { 0 };
    std::fill_n(output, 64, -1);
    core.recordReadBuffer(cmdStream, inputBuffer.getHandle(), &output);
    core.submitCommandStream(cmdStream);
    std::cout << "[";
    for (int i = 0; i < 64; i++) {
        std::cout << output[i] << ", ";
    }
    std::cout << "]" << std::endl;
    }
    return 0;
}

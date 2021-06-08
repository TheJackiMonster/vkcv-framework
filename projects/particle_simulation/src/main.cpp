#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>

uint32_t findMemoryType( vk::PhysicalDeviceMemoryProperties const & memoryProperties,
                         uint32_t                                   typeBits,
                         vk::MemoryPropertyFlags                    requirementsMask )
{
    uint32_t typeIndex = uint32_t( ~0 );
    for ( uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++ )
    {
        if ( ( typeBits & 1 ) &&
             ( ( memoryProperties.memoryTypes[i].propertyFlags & requirementsMask ) == requirementsMask ) )
        {
            typeIndex = i;
            break;
        }
        typeBits >>= 1;
    }
    assert( typeIndex != uint32_t( ~0 ) );
    return typeIndex;
}

int main(int argc, const char** argv) {
    const char* applicationName = "Particlesystem";

    uint32_t windowWidth = 800;
    uint32_t windowHeight = 600;
    vkcv::Window window = vkcv::Window::create(
            applicationName,
            windowWidth,
            windowHeight,
            true
    );

    vkcv::CameraManager cameraManager(window, windowWidth, windowHeight);

    window.initEvents();

    vkcv::Core core = vkcv::Core::create(
            window,
            applicationName,
            VK_MAKE_VERSION(0, 0, 1),
            { vk::QueueFlagBits::eTransfer,vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute },
            {},
            { "VK_KHR_swapchain" }
    );

    const auto& context = core.getContext();
    const vk::Instance& instance = context.getInstance();
    const vk::PhysicalDevice& physicalDevice = context.getPhysicalDevice();
    const vk::Device& device = context.getDevice();

    struct vec3 {
        float x, y, z;
    };

    const size_t n = 5027;

    auto testBuffer = core.createBuffer<vec3>(vkcv::BufferType::VERTEX, n, vkcv::BufferMemoryType::DEVICE_LOCAL);
    vec3 vec_data[n];

    for (size_t i = 0; i < n; i++) {
        vec_data[i] = { 42, static_cast<float>(i), 7 };
    }

    testBuffer.fill(vec_data);

    auto particleIndexBuffer = core.createBuffer<uint16_t>(vkcv::BufferType::INDEX, n, vkcv::BufferMemoryType::DEVICE_LOCAL);
    uint16_t indices[3] = { 0, 1, 2 };
    particleIndexBuffer.fill(&indices[0], sizeof(indices));

    std::cout << "Physical device: " << physicalDevice.getProperties().deviceName << std::endl;

    // an example attachment for passes that output to the window
    const vkcv::AttachmentDescription present_color_attachment(
            vkcv::AttachmentOperation::STORE,
            vkcv::AttachmentOperation::CLEAR,
            core.getSwapchainImageFormat());


    vkcv::PassConfig particlePassDefinition({ present_color_attachment });
    vkcv::PassHandle particlePass = core.createPass(particlePassDefinition);

    if (!particlePass)
    {
        std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
        return EXIT_FAILURE;
    }

    vkcv::ShaderProgram particleShaderProgram{};
    particleShaderProgram.addShader(vkcv::ShaderStage::VERTEX, std::filesystem::path("shaders/vert.spv"));
    particleShaderProgram.addShader(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("shaders/frag.spv"));
    particleShaderProgram.reflectShader(vkcv::ShaderStage::VERTEX);
    particleShaderProgram.reflectShader(vkcv::ShaderStage::FRAGMENT);

    std::vector<vkcv::DescriptorBinding> descriptorBindings = {
            vkcv::DescriptorBinding(vkcv::DescriptorType::UNIFORM_BUFFER,   1, vkcv::ShaderStage::FRAGMENT),
            vkcv::DescriptorBinding(vkcv::DescriptorType::UNIFORM_BUFFER,   1, vkcv::ShaderStage::FRAGMENT)};
    vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(descriptorBindings);

    const vkcv::PipelineConfig particlePipelineDefinition(
            particleShaderProgram,
            UINT32_MAX,
            UINT32_MAX,
            particlePass,
            {},
            { core.getDescriptorSet(descriptorSet).layout },
            true);

    vkcv::PipelineHandle particlePipeline = core.createGraphicsPipeline(particlePipelineDefinition);
    vkcv::Buffer<glm::vec4> color = core.createBuffer<glm::vec4>(
            vkcv::BufferType::UNIFORM,
            1
            );

    vkcv::Buffer<glm::vec2> position = core.createBuffer<glm::vec2>(
            vkcv::BufferType::UNIFORM,
            1
    );

    vkcv::DescriptorWrites setWrites;
    setWrites.uniformBufferWrites = {vkcv::UniformBufferDescriptorWrite(0,color.getHandle()),
                                     vkcv::UniformBufferDescriptorWrite(1,position.getHandle())};
    core.writeResourceDescription(descriptorSet,0,setWrites);

    if (!particlePipeline)
    {
        std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
        return EXIT_FAILURE;
    }

    const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

    const vkcv::Mesh renderMesh({}, particleIndexBuffer.getVulkanHandle(), 3);
    vkcv::DrawcallInfo drawcalls(renderMesh, {vkcv::DescriptorSetUsage(0, core.getDescriptorSet(descriptorSet).vulkanHandle)});

    auto start = std::chrono::system_clock::now();

    glm::vec4 colorData = glm::vec4(1.0f,1.0f,0.0f,1.0f);

    glm::vec2 pos = glm::vec2(1.f);

    window.e_mouseMove.add([&]( double offsetX, double offsetY) {
        pos = glm::vec2(static_cast<float>(offsetX), static_cast<float>(offsetY));
    });

    while (window.isWindowOpen())
    {
        window.pollEvents();

        uint32_t swapchainWidth, swapchainHeight;
        if (!core.beginFrame(swapchainWidth, swapchainHeight)) {
            continue;
        }

        color.fill(&colorData);

        position.fill(&pos);

        auto end = std::chrono::system_clock::now();
        auto deltatime = end - start;
        start = end;
        cameraManager.getCamera().updateView(std::chrono::duration<double>(deltatime).count());
        const glm::mat4 mvp = cameraManager.getCamera().getProjection() * cameraManager.getCamera().getView();

        vkcv::PushConstantData pushConstantData((void*)&mvp, sizeof(glm::mat4));
        auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

        core.recordDrawcallsToCmdStream(
                cmdStream,
                particlePass,
                particlePipeline,
                pushConstantData,
                {drawcalls},
                {swapchainInput});
        core.prepareSwapchainImageForPresent(cmdStream);
        core.submitCommandStream(cmdStream);
        core.endFrame();
    }

    return 0;
}

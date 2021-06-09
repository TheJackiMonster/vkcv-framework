#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>

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

    auto particleIndexBuffer = core.createBuffer<uint16_t>(vkcv::BufferType::INDEX, 3, vkcv::BufferMemoryType::DEVICE_LOCAL);
    uint16_t indices[3] = { 0, 1, 2 };
    particleIndexBuffer.fill(&indices[0], sizeof(indices));


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

    vkcv::Buffer<glm::vec3> vertexbuffer = core.createBuffer<glm::vec3>(
            vkcv::BufferType::VERTEX,
            3
    );

    const std::vector<glm::vec3> vertices = {glm::vec3(-0.5, 0.5, -1),
                                             glm::vec3( 0.5, 0.5, -1),
                                             glm::vec3(0, -0.5, -1)};

    vertexbuffer.fill(vertices);


    vkcv::VertexAttribute attrib = vkcv::VertexAttribute{
            vkcv::PrimitiveType::POSITION,
            0,
            sizeof(glm::vec3) * vertices.size(),
            0,
            5126,
            3};


    std::vector<vkcv::DescriptorBinding> descriptorBindings = {
            vkcv::DescriptorBinding(vkcv::DescriptorType::UNIFORM_BUFFER,   1, vkcv::ShaderStage::FRAGMENT),
            vkcv::DescriptorBinding(vkcv::DescriptorType::UNIFORM_BUFFER,   1, vkcv::ShaderStage::FRAGMENT)};
    vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(descriptorBindings);

    const vkcv::PipelineConfig particlePipelineDefinition(
            particleShaderProgram,
            UINT32_MAX,
            UINT32_MAX,
            particlePass,
            {attrib},
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

    const std::vector<vkcv::VertexBufferBinding> vertexBufferBindings = {
            vkcv::VertexBufferBinding(0, vertexbuffer.getVulkanHandle())
    };

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

    const vkcv::Mesh renderMesh({vertexBufferBindings}, particleIndexBuffer.getVulkanHandle(), 3);
    vkcv::DescriptorSetUsage    descriptorUsage(0, core.getDescriptorSet(descriptorSet).vulkanHandle);
    vkcv::DrawcallInfo drawcalls(renderMesh, {vkcv::DescriptorSetUsage(0, core.getDescriptorSet(descriptorSet).vulkanHandle)});

    auto start = std::chrono::system_clock::now();

    glm::vec4 colorData = glm::vec4(1.0f,1.0f,0.0f,1.0f);

    glm::vec2 pos = glm::vec2(1.f);

    window.e_mouseMove.add([&]( double offsetX, double offsetY) {
        pos = glm::vec2(static_cast<float>(offsetX), static_cast<float>(offsetY));
    });


    struct Particle{
        glm::vec2 Position;
        glm::vec2 Velocity;
        float Rotation = 0.0f;
        float SizeBegin, SizeEnd;

        float LifeTime = 1.0f;
        float LifeRemaining = 0.0f;

        bool Active = true;
    };

    std::vector<Particle> m_ParticlePool;
    uint32_t poolIndex = 999;

    m_ParticlePool.resize(1000);

    //float angle = 0.0005;
    glm::mat4 modelmatrix = glm::mat4(1.0);

    for(auto& particle : m_ParticlePool){
        if(!particle.Active){
            continue;
        }
    }

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
        float deltatime = std::chrono::duration<float>(end - start).count();
        start = end;

        //modelmatrix = glm::rotate(modelmatrix, angle, glm::vec3(0,0,1));
        for(auto& particle : m_ParticlePool){
            if (!particle.Active){
                continue;
            }
            if (particle.LifeRemaining <= 0.0f){
                particle.Active = false;
                continue;
            }

            particle.LifeRemaining -= deltatime;
            particle.Position += particle.Velocity * deltatime;
            particle.Rotation += 0.01f * deltatime;

        }

        cameraManager.getCamera().updateView(deltatime);
        const glm::mat4 mvp = modelmatrix * cameraManager.getCamera().getProjection() * cameraManager.getCamera().getView();

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

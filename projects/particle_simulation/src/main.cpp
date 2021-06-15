#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include "ParticleSystem.hpp"
#include <random>

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
    cameraManager.getCamera().setNearFar(0.000000001f, 10.f);

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

    vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(particleShaderProgram.getReflectedDescriptors()[0]);

    vkcv::Buffer<glm::vec3> vertexBuffer = core.createBuffer<glm::vec3>(
            vkcv::BufferType::VERTEX,
            3
    );
    const std::vector<vkcv::VertexAttachment> vertexAttachments = particleShaderProgram.getVertexAttachments();

    std::vector<vkcv::VertexBinding> bindings;
    for (size_t i = 0; i < vertexAttachments.size(); i++) {
        bindings.push_back(vkcv::VertexBinding(i, { vertexAttachments[i] }));
    }

    const vkcv::VertexLayout particleLayout(bindings);

    const vkcv::PipelineConfig particlePipelineDefinition(
            particleShaderProgram,
            UINT32_MAX,
            UINT32_MAX,
            particlePass,
            {particleLayout},
            { core.getDescriptorSet(descriptorSet).layout },
            true);

    const std::vector<glm::vec3> vertices = {glm::vec3(-0.02, 0.02, 0),
                                             glm::vec3( 0.02, 0.02, 0),
                                             glm::vec3(0, -0.02, 0)};

    vertexBuffer.fill(vertices);

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
            vkcv::VertexBufferBinding(0, vertexBuffer.getVulkanHandle())
    };


    vkcv::DescriptorWrites setWrites;
    setWrites.uniformBufferWrites = {vkcv::UniformBufferDescriptorWrite(0,color.getHandle()),
                                     vkcv::UniformBufferDescriptorWrite(1,position.getHandle())};
    core.writeDescriptorSet(descriptorSet, setWrites);

    if (!particlePipeline)
    {
        std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
        return EXIT_FAILURE;
    }

    const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

    const vkcv::Mesh renderMesh({vertexBufferBindings}, particleIndexBuffer.getVulkanHandle(), particleIndexBuffer.getCount());
    vkcv::DescriptorSetUsage    descriptorUsage(0, core.getDescriptorSet(descriptorSet).vulkanHandle);
    //vkcv::DrawcallInfo drawcalls(renderMesh, {vkcv::DescriptorSetUsage(0, core.getDescriptorSet(descriptorSet).vulkanHandle)});

    glm::vec2 pos = glm::vec2(1.f);

    window.e_mouseMove.add([&]( double offsetX, double offsetY) {
        pos = glm::vec2(static_cast<float>(offsetX), static_cast<float>(offsetY));
    });

    glm::vec3 minVelocity = glm::vec3(-0.5f,-0.5f,0.f);
    glm::vec3 maxVelocity = glm::vec3(0.5f,0.5f,0.f);
    glm::vec2 lifeTime = glm::vec2(0.f,5.f);
    ParticleSystem particleSystem = ParticleSystem( 100 , minVelocity, maxVelocity, lifeTime);

    std::vector<glm::mat4> modelMatrices;
    std::vector<vkcv::DrawcallInfo> drawcalls;
    for(auto particle :  particleSystem.getParticles()) {
        modelMatrices.push_back(glm::translate(glm::mat4(1.f), particle.getPosition()));
        drawcalls.push_back(vkcv::DrawcallInfo(renderMesh, {descriptorUsage}));
    }

    auto start = std::chrono::system_clock::now();

    glm::vec4 colorData = glm::vec4(1.0f,1.0f,0.0f,1.0f);

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
        particleSystem.updateParticles( deltatime );

        modelMatrices.clear();
        for(Particle particle :  particleSystem.getParticles()) {
            modelMatrices.push_back(glm::translate(glm::mat4(1.f), particle.getPosition()));
        }

        //modelmatrix = glm::rotate(modelmatrix, angle, glm::vec3(0,0,1));

        cameraManager.getCamera().updateView(deltatime);
        //const glm::mat4 mvp = modelmatrix * cameraManager.getCamera().getProjection() * cameraManager.getCamera().getView();
        std::vector<glm::mat4> mvp;
        mvp.clear();
        for(const auto& m : modelMatrices){
            mvp.push_back(m * cameraManager.getCamera().getProjection() * cameraManager.getCamera().getView());
        }

        vkcv::PushConstantData pushConstantData((void*)mvp.data(), sizeof(glm::mat4));
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

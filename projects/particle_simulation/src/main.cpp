#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include "ParticleSystem.hpp"
#include <random>
#include <glm/gtc/matrix_access.hpp>
#include <time.h>
#include <vkcv/shader/GLSLCompiler.hpp>
#include "BloomAndFlares.hpp"

int main(int argc, const char **argv) {
    const char *applicationName = "Particlesystem";

    uint32_t windowWidth = 800;
    uint32_t windowHeight = 600;
    vkcv::Window window = vkcv::Window::create(
            applicationName,
            windowWidth,
            windowHeight,
            true
    );

    vkcv::camera::CameraManager cameraManager(window);

    vkcv::Core core = vkcv::Core::create(
            window,
            applicationName,
            VK_MAKE_VERSION(0, 0, 1),
            {vk::QueueFlagBits::eTransfer, vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute},
            {},
            {"VK_KHR_swapchain"}
    );

    auto particleIndexBuffer = core.createBuffer<uint16_t>(vkcv::BufferType::INDEX, 3,
                                                           vkcv::BufferMemoryType::DEVICE_LOCAL);
    uint16_t indices[3] = {0, 1, 2};
    particleIndexBuffer.fill(&indices[0], sizeof(indices));

    vk::Format colorFormat = vk::Format::eR16G16B16A16Sfloat;
    // an example attachment for passes that output to the window
    const vkcv::AttachmentDescription present_color_attachment(
            vkcv::AttachmentOperation::STORE,
            vkcv::AttachmentOperation::CLEAR,
            colorFormat);


    vkcv::PassConfig particlePassDefinition({present_color_attachment});
    vkcv::PassHandle particlePass = core.createPass(particlePassDefinition);

    vkcv::PassConfig computePassDefinition({});
    vkcv::PassHandle computePass = core.createPass(computePassDefinition);

    if (!particlePass || !computePass)
    {
        std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
        return EXIT_FAILURE;
    }

    // use space or use water or gravity
    std::string shaderPathCompute = "shaders/shader_space.comp";
	std::string shaderPathFragment = "shaders/shader_space.frag";
    
    for (int i = 1; i < argc; i++) {
    	if (strcmp(argv[i], "--space") == 0) {
    		shaderPathCompute = "shaders/shader_space.comp";
			shaderPathFragment = "shaders/shader_space.frag";
    	} else
		if (strcmp(argv[i], "--water") == 0) {
			shaderPathCompute = "shaders/shader_water.comp";
			shaderPathFragment = "shaders/shader_water.frag";
		} else
		if (strcmp(argv[i], "--gravity") == 0) {
			shaderPathCompute = "shaders/shader_gravity.comp";
			shaderPathFragment = "shaders/shader_space.frag";
		}
    }

    vkcv::shader::GLSLCompiler compiler;
    vkcv::ShaderProgram computeShaderProgram{};
    compiler.compile(vkcv::ShaderStage::COMPUTE, shaderPathCompute, [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
        computeShaderProgram.addShader(shaderStage, path);
    });

    vkcv::DescriptorSetHandle computeDescriptorSet = core.createDescriptorSet(computeShaderProgram.getReflectedDescriptors()[0]);

    const std::vector<vkcv::VertexAttachment> computeVertexAttachments = computeShaderProgram.getVertexAttachments();

    std::vector<vkcv::VertexBinding> computeBindings;
    for (size_t i = 0; i < computeVertexAttachments.size(); i++) {
        computeBindings.push_back(vkcv::VertexBinding(i, { computeVertexAttachments[i] }));
    }
    const vkcv::VertexLayout computeLayout(computeBindings);

    vkcv::ShaderProgram particleShaderProgram{};
    compiler.compile(vkcv::ShaderStage::VERTEX, "shaders/shader.vert", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
        particleShaderProgram.addShader(shaderStage, path);
    });
    compiler.compile(vkcv::ShaderStage::FRAGMENT, shaderPathFragment, [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
        particleShaderProgram.addShader(shaderStage, path);
    });

    vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(
            particleShaderProgram.getReflectedDescriptors()[0]);

    vkcv::Buffer<glm::vec3> vertexBuffer = core.createBuffer<glm::vec3>(
            vkcv::BufferType::VERTEX,
            3
    );
    const std::vector<vkcv::VertexAttachment> vertexAttachments = particleShaderProgram.getVertexAttachments();

    const std::vector<vkcv::VertexBufferBinding> vertexBufferBindings = {
            vkcv::VertexBufferBinding(0, vertexBuffer.getVulkanHandle())};

    std::vector<vkcv::VertexBinding> bindings;
    for (size_t i = 0; i < vertexAttachments.size(); i++) {
        bindings.push_back(vkcv::VertexBinding(i, {vertexAttachments[i]}));
    }

    const vkcv::VertexLayout particleLayout(bindings);

    vkcv::PipelineConfig particlePipelineDefinition{
            particleShaderProgram,
            UINT32_MAX,
            UINT32_MAX,
            particlePass,
            {particleLayout},
            {core.getDescriptorSet(descriptorSet).layout},
            true};
    particlePipelineDefinition.m_blendMode = vkcv::BlendMode::Additive;

    const std::vector<glm::vec3> vertices = {glm::vec3(-0.012, 0.012, 0),
                                             glm::vec3(0.012, 0.012, 0),
                                             glm::vec3(0, -0.012, 0)};

    vertexBuffer.fill(vertices);

    vkcv::PipelineHandle particlePipeline = core.createGraphicsPipeline(particlePipelineDefinition);

    vkcv::PipelineHandle computePipeline = core.createComputePipeline(computeShaderProgram, {core.getDescriptorSet(computeDescriptorSet).layout} );

    vkcv::Buffer<glm::vec4> color = core.createBuffer<glm::vec4>(
            vkcv::BufferType::UNIFORM,
            1
    );

    vkcv::Buffer<glm::vec2> position = core.createBuffer<glm::vec2>(
            vkcv::BufferType::UNIFORM,
            1
    );

    glm::vec3 minVelocity = glm::vec3(-0.1f,-0.1f,-0.1f);
    glm::vec3 maxVelocity = glm::vec3(0.1f,0.1f,0.1f);
    glm::vec2 lifeTime = glm::vec2(-1.f,8.f);
    ParticleSystem particleSystem = ParticleSystem( 100000 , minVelocity, maxVelocity, lifeTime);

    vkcv::Buffer<Particle> particleBuffer = core.createBuffer<Particle>(
            vkcv::BufferType::STORAGE,
            particleSystem.getParticles().size()
    );

    particleBuffer.fill(particleSystem.getParticles());

    vkcv::DescriptorWrites setWrites;
    setWrites.uniformBufferWrites = {vkcv::UniformBufferDescriptorWrite(0,color.getHandle()),
                                     vkcv::UniformBufferDescriptorWrite(1,position.getHandle())};
    setWrites.storageBufferWrites = { vkcv::StorageBufferDescriptorWrite(2,particleBuffer.getHandle())};
    core.writeDescriptorSet(descriptorSet, setWrites);

    vkcv::DescriptorWrites computeWrites;
    computeWrites.storageBufferWrites = { vkcv::StorageBufferDescriptorWrite(0,particleBuffer.getHandle())};
    core.writeDescriptorSet(computeDescriptorSet, computeWrites);

    if (!particlePipeline || !computePipeline)
    {
        std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
        return EXIT_FAILURE;
    }

    const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

    const vkcv::Mesh renderMesh({vertexBufferBindings}, particleIndexBuffer.getVulkanHandle(),
                                particleIndexBuffer.getCount());
    vkcv::DescriptorSetUsage descriptorUsage(0, core.getDescriptorSet(descriptorSet).vulkanHandle);

    auto pos = glm::vec2(0.f);
    auto spawnPosition = glm::vec3(0.f);

    window.e_mouseMove.add([&](double offsetX, double offsetY) {
        pos = glm::vec2(static_cast<float>(offsetX), static_cast<float>(offsetY));
        pos.x = (-2 * pos.x + static_cast<float>(window.getWidth())) / static_cast<float>(window.getWidth());
        pos.y = (-2 * pos.y + static_cast<float>(window.getHeight())) / static_cast<float>(window.getHeight());
        spawnPosition = glm::vec3(pos.x, pos.y, 0.f);
        particleSystem.setRespawnPos(glm::vec3(-spawnPosition.x, spawnPosition.y, spawnPosition.z));
    });

    std::vector<glm::mat4> modelMatrices;
    std::vector<vkcv::DrawcallInfo> drawcalls;
    drawcalls.push_back(vkcv::DrawcallInfo(renderMesh, {descriptorUsage}, particleSystem.getParticles().size()));

    auto start = std::chrono::system_clock::now();

    glm::vec4 colorData = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
    uint32_t camIndex0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
    uint32_t camIndex1 = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);

    cameraManager.getCamera(camIndex0).setNearFar(0.1, 30);
    cameraManager.getCamera(camIndex1).setNearFar(0.1, 30);

    cameraManager.setActiveCamera(1);

    cameraManager.getCamera(camIndex0).setPosition(glm::vec3(0, 0, -2));
    cameraManager.getCamera(camIndex1).setPosition(glm::vec3(0.0f, 0.0f, -2.0f));
    cameraManager.getCamera(camIndex1).setCenter(glm::vec3(0.0f, 0.0f, 0.0f));

    vkcv::ImageHandle colorBuffer = core.createImage(colorFormat, windowWidth, windowHeight, 1, false, true, true).getHandle();
    BloomAndFlares bloomAndFlares(&core, colorFormat, windowWidth, windowHeight);
    window.e_resize.add([&](int width, int height) {
        windowWidth = width;
        windowHeight = height;
        colorBuffer = core.createImage(colorFormat, windowWidth, windowHeight, 1, false, true, true).getHandle();
        bloomAndFlares.updateImageDimensions(width, height);
    });

    vkcv::ShaderProgram tonemappingShader;
    compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/tonemapping.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
        tonemappingShader.addShader(shaderStage, path);
    });

    vkcv::DescriptorSetHandle tonemappingDescriptor = core.createDescriptorSet(tonemappingShader.getReflectedDescriptors()[0]);
    vkcv::PipelineHandle tonemappingPipe = core.createComputePipeline(
        tonemappingShader, 
        { core.getDescriptorSet(tonemappingDescriptor).layout });

    std::uniform_real_distribution<float> rdm = std::uniform_real_distribution<float>(0.95f, 1.05f);
    std::default_random_engine rdmEngine;
    while (window.isWindowOpen()) {
        vkcv::Window::pollEvents();

        uint32_t swapchainWidth, swapchainHeight;
        if (!core.beginFrame(swapchainWidth, swapchainHeight)) {
            continue;
        }

        color.fill(&colorData);
        position.fill(&pos);

        auto end = std::chrono::system_clock::now();
        float deltatime = 0.000001 * static_cast<float>( std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() );
        start = end;

        cameraManager.update(deltatime);

        // split view and projection to allow for easy billboarding in shader
        struct {
			glm::mat4 view;
			glm::mat4 projection;
        } renderingMatrices;
        
        renderingMatrices.view = cameraManager.getActiveCamera().getView();
        renderingMatrices.projection = cameraManager.getActiveCamera().getProjection();

        auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);
        float random = rdm(rdmEngine);
        glm::vec2 pushData = glm::vec2(deltatime, random);

        vkcv::PushConstants pushConstantsCompute (sizeof(glm::vec2));
        pushConstantsCompute.appendDrawcall(pushData);
        
        uint32_t computeDispatchCount[3] = {static_cast<uint32_t> (std::ceil(particleSystem.getParticles().size()/256.f)),1,1};
        core.recordComputeDispatchToCmdStream(cmdStream,
                                              computePipeline,
                                              computeDispatchCount,
                                              {vkcv::DescriptorSetUsage(0,core.getDescriptorSet(computeDescriptorSet).vulkanHandle)},
											  pushConstantsCompute);

        core.recordBufferMemoryBarrier(cmdStream, particleBuffer.getHandle());

        vkcv::PushConstants pushConstantsDraw (sizeof(renderingMatrices));
        pushConstantsDraw.appendDrawcall(renderingMatrices);
        
        core.recordDrawcallsToCmdStream(
                cmdStream,
                particlePass,
                particlePipeline,
				pushConstantsDraw,
                {drawcalls},
                { colorBuffer });

        bloomAndFlares.execWholePipeline(cmdStream, colorBuffer);

        core.prepareImageForStorage(cmdStream, colorBuffer);
        core.prepareImageForStorage(cmdStream, swapchainInput);

        vkcv::DescriptorWrites tonemappingDescriptorWrites;
        tonemappingDescriptorWrites.storageImageWrites = {
            vkcv::StorageImageDescriptorWrite(0, colorBuffer),
            vkcv::StorageImageDescriptorWrite(1, swapchainInput)
        };
        core.writeDescriptorSet(tonemappingDescriptor, tonemappingDescriptorWrites);

        uint32_t tonemappingDispatchCount[3];
        tonemappingDispatchCount[0] = std::ceil(windowWidth / 8.f);
        tonemappingDispatchCount[1] = std::ceil(windowHeight / 8.f);
        tonemappingDispatchCount[2] = 1;

        core.recordComputeDispatchToCmdStream(
            cmdStream, 
            tonemappingPipe, 
            tonemappingDispatchCount, 
            {vkcv::DescriptorSetUsage(0, core.getDescriptorSet(tonemappingDescriptor).vulkanHandle) },
            vkcv::PushConstants(0));

        core.prepareSwapchainImageForPresent(cmdStream);
        core.submitCommandStream(cmdStream);
        core.endFrame();
    }

    return 0;
}
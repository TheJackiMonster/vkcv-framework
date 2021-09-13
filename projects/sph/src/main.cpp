#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include <random>
#include <glm/gtc/matrix_access.hpp>
#include <time.h>
#include <vkcv/shader/GLSLCompiler.hpp>
#include "BloomAndFlares.hpp"
#include "PipelineInit.hpp"
#include "Particle.hpp"

int main(int argc, const char **argv) {
    const char *applicationName = "SPH";

    vkcv::Window window = vkcv::Window::create(
            applicationName,
            800,
            600,
            true
    );

    vkcv::camera::CameraManager cameraManager(window);
	
    vkcv::Core core = vkcv::Core::create(
            window,
            applicationName,
            VK_MAKE_VERSION(0, 0, 1),
            {vk::QueueFlagBits::eTransfer, vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute},
			{ VK_KHR_SWAPCHAIN_EXTENSION_NAME }
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
	vkcv::shader::GLSLCompiler compiler;

// comp shader 1
    vkcv::PipelineHandle computePipeline1;
    vkcv::DescriptorSetHandle computeDescriptorSet1 = PipelineInit::ComputePipelineInit(&core, vkcv::ShaderStage::COMPUTE,
                                                                          "shaders/pressure.comp", computePipeline1);
// comp shader 2
    vkcv::PipelineHandle computePipeline2;
    vkcv::DescriptorSetHandle computeDescriptorSet2 = PipelineInit::ComputePipelineInit(&core, vkcv::ShaderStage::COMPUTE,
                                                                          "shaders/force.comp", computePipeline2);

//comp shader 3
    vkcv::PipelineHandle computePipeline3;
    vkcv::DescriptorSetHandle computeDescriptorSet3 = PipelineInit::ComputePipelineInit(&core, vkcv::ShaderStage::COMPUTE,
                                                                           "shaders/updateData.comp", computePipeline3);

//comp shader 4
    vkcv::PipelineHandle computePipeline4;
    vkcv::DescriptorSetHandle computeDescriptorSet4 = PipelineInit::ComputePipelineInit(&core, vkcv::ShaderStage::COMPUTE,
                                                                            "shaders/flip.comp", computePipeline4);

// shader
    vkcv::ShaderProgram particleShaderProgram{};
    compiler.compile(vkcv::ShaderStage::VERTEX, "shaders/shader.vert", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
        particleShaderProgram.addShader(shaderStage, path);
    });
    compiler.compile(vkcv::ShaderStage::FRAGMENT, "shaders/shader_water.frag", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
        particleShaderProgram.addShader(shaderStage, path);
    });

    vkcv::DescriptorSetLayoutHandle descriptorSetLayout = core.createDescriptorSetLayout(
            particleShaderProgram.getReflectedDescriptors().at(0));
    vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(descriptorSetLayout);

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
            {core.getDescriptorSetLayout(descriptorSetLayout).vulkanHandle},
            true};
    particlePipelineDefinition.m_blendMode = vkcv::BlendMode::Additive;

    const std::vector<glm::vec3> vertices = {glm::vec3(-0.012, 0.012, 0),
                                             glm::vec3(0.012, 0.012, 0),
                                             glm::vec3(0, -0.012, 0)};

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

    int numberParticles = 10000;
    std::vector<Particle> particles;
    for (int i = 0; i < numberParticles; i++) {
        const float lo = 0.4;
        const float hi = 0.6;
        const float vlo = 0.4;
        const float vhi = 0.6;
        float x = lo + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(hi-lo)));
        float y = lo + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(hi-lo)));
        float z = lo + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(hi-lo)));
        float vx = vlo + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(vhi-vlo)));
        float vy = vlo + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(vhi-vlo)));
        float vz = vlo + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(vhi-vlo)));
        glm::vec3 pos = glm::vec3(x,y,z);
        glm::vec3 vel = glm::vec3(vx,vy,vz);
        particles.push_back(Particle(pos, vel));
    }

    vkcv::Buffer<Particle> particleBuffer1 = core.createBuffer<Particle>(
            vkcv::BufferType::STORAGE,
            numberParticles * sizeof(glm::vec4) * 3

    );

    vkcv::Buffer<Particle> particleBuffer2 = core.createBuffer<Particle>(
        vkcv::BufferType::STORAGE,
        numberParticles * sizeof(glm::vec4) * 3
    );

    particleBuffer1.fill(particles);
	particleBuffer2.fill(particles);

    vkcv::DescriptorWrites setWrites;
    setWrites.uniformBufferWrites = {vkcv::BufferDescriptorWrite(0,color.getHandle()),
                                     vkcv::BufferDescriptorWrite(1,position.getHandle())};
    setWrites.storageBufferWrites = { vkcv::BufferDescriptorWrite(2,particleBuffer1.getHandle()),
									  vkcv::BufferDescriptorWrite(3,particleBuffer2.getHandle())};
    core.writeDescriptorSet(descriptorSet, setWrites);

    vkcv::DescriptorWrites computeWrites;
    computeWrites.storageBufferWrites = { vkcv::BufferDescriptorWrite(0,particleBuffer1.getHandle()),
										  vkcv::BufferDescriptorWrite(1,particleBuffer2.getHandle())};
    core.writeDescriptorSet(computeDescriptorSet1, computeWrites);
	core.writeDescriptorSet(computeDescriptorSet2, computeWrites);

    if (!particlePipeline || !computePipeline1 || !computePipeline2 || !computePipeline3 || !computePipeline4)
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

    std::vector<glm::mat4> modelMatrices;
    std::vector<vkcv::DrawcallInfo> drawcalls;
    drawcalls.push_back(vkcv::DrawcallInfo(renderMesh, {descriptorUsage}, numberParticles * sizeof(glm::vec4) * 3));

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

	auto swapchainExtent = core.getSwapchain().getExtent();
	
    vkcv::ImageHandle colorBuffer = core.createImage(
			colorFormat,
			swapchainExtent.width,
			swapchainExtent.height,
			1, false, true, true
	).getHandle();
    BloomAndFlares bloomAndFlares(&core, colorFormat, swapchainExtent.width, swapchainExtent.height);
    window.e_resize.add([&](int width, int height) {
		swapchainExtent = core.getSwapchain().getExtent();
        colorBuffer = core.createImage(
				colorFormat,
				swapchainExtent.width,
				swapchainExtent.height,
				1, false, true, true
		).getHandle();
        bloomAndFlares.updateImageDimensions(width, height);
    });

    //tone mapping shader & pipeline
    vkcv::PipelineHandle tonemappingPipe;
    vkcv::DescriptorSetHandle tonemappingDescriptor = PipelineInit::ComputePipelineInit(&core, vkcv::ShaderStage::COMPUTE,
                                                                                        "shaders/tonemapping.comp", tonemappingPipe);


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

        //deltatime wird noch nicht genutzt
        glm::vec2 pushData = glm::vec2(deltatime, (float)numberParticles);

        vkcv::PushConstants pushConstantsCompute (sizeof(glm::vec2));
        pushConstantsCompute.appendDrawcall(pushData);

        uint32_t computeDispatchCount[3] = {static_cast<uint32_t> (std::ceil(numberParticles/256.f)),1,1};
        core.recordComputeDispatchToCmdStream(cmdStream,
                                              computePipeline1,
                                              computeDispatchCount,
                                              {vkcv::DescriptorSetUsage(0,core.getDescriptorSet(computeDescriptorSet1).vulkanHandle)},
											  pushConstantsCompute);

        core.recordBufferMemoryBarrier(cmdStream, particleBuffer1.getHandle());
		core.recordBufferMemoryBarrier(cmdStream, particleBuffer2.getHandle());

		core.recordComputeDispatchToCmdStream(cmdStream,
											  computePipeline2,
											  computeDispatchCount,
											  {vkcv::DescriptorSetUsage(0,core.getDescriptorSet(computeDescriptorSet2).vulkanHandle)},
											  pushConstantsCompute);

		core.recordBufferMemoryBarrier(cmdStream, particleBuffer1.getHandle());
		core.recordBufferMemoryBarrier(cmdStream, particleBuffer2.getHandle());

        core.recordComputeDispatchToCmdStream(cmdStream,
            computePipeline3,
            computeDispatchCount,
            { vkcv::DescriptorSetUsage(0,core.getDescriptorSet(computeDescriptorSet3).vulkanHandle) },
            pushConstantsCompute);

        core.recordBufferMemoryBarrier(cmdStream, particleBuffer1.getHandle());
        core.recordBufferMemoryBarrier(cmdStream, particleBuffer2.getHandle());

        core.recordComputeDispatchToCmdStream(cmdStream,
            computePipeline4,
            computeDispatchCount,
            { vkcv::DescriptorSetUsage(0,core.getDescriptorSet(computeDescriptorSet4).vulkanHandle) },
            pushConstantsCompute);

        core.recordBufferMemoryBarrier(cmdStream, particleBuffer1.getHandle());
        core.recordBufferMemoryBarrier(cmdStream, particleBuffer2.getHandle());


        //bloomAndFlares & tonemapping
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
        tonemappingDispatchCount[0] = std::ceil(swapchainExtent.width / 8.f);
        tonemappingDispatchCount[1] = std::ceil(swapchainExtent.height / 8.f);
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

#include <iostream>
#include <vkcv/Core.hpp>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include <random>
#include <time.h>
#include <vkcv/shader/GLSLCompiler.hpp>
#include "BloomAndFlares.hpp"
#include "PipelineInit.hpp"
#include "Particle.hpp"

int main(int argc, const char **argv) {
    const char *applicationName = "SPH";

    // creating core object that will handle all vulkan objects
    vkcv::Core core = vkcv::Core::create(
        applicationName,
        VK_MAKE_VERSION(0, 0, 1),
        { vk::QueueFlagBits::eTransfer, vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute },
        { VK_KHR_SWAPCHAIN_EXTENSION_NAME }
    );

    // creating window
    vkcv::WindowHandle windowHandle = core.createWindow(applicationName, 1920, 1080, false);
    vkcv::Window& window = core.getWindow(windowHandle);

    vkcv::camera::CameraManager cameraManager(window);

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

    //rotation
    float rotationx = 0;
    float rotationy = 0;

    // params  
    float param_h = 0.20;
    float param_mass = 0.03;
    float param_gasConstant = 3500;
    float param_offset = 200;
    float param_gravity = -5000;
    float param_viscosity = 10;
    float param_ABSORBTION = 0.5;
    float param_dt = 0.0005;

    if (!particlePass || !computePass)
    {
        std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
        return EXIT_FAILURE;
    }
	vkcv::shader::GLSLCompiler compiler;

    // pressure shader --> computes the pressure for all particles
    vkcv::ComputePipelineHandle pressurePipeline;
    vkcv::DescriptorSetHandle pressureDescriptorSet = PipelineInit::ComputePipelineInit(&core, vkcv::ShaderStage::COMPUTE,
                                                                                        "shaders/pressure.comp", pressurePipeline);
    // force shader --> computes the force that effects the particles
    vkcv::ComputePipelineHandle forcePipeline;
    vkcv::DescriptorSetHandle forceDescriptorSet = PipelineInit::ComputePipelineInit(&core, vkcv::ShaderStage::COMPUTE,
                                                                                     "shaders/force.comp", forcePipeline);

    // update data shader --> applies the force on all particles and updates their position
    vkcv::ComputePipelineHandle updateDataPipeline;
    vkcv::DescriptorSetHandle updateDataDescriptorSet = PipelineInit::ComputePipelineInit(&core, vkcv::ShaderStage::COMPUTE,
                                                                                          "shaders/updateData.comp", updateDataPipeline);

    // flip shader --> flips input and output buffer
    vkcv::ComputePipelineHandle flipPipeline;
    vkcv::DescriptorSetHandle flipDescriptorSet = PipelineInit::ComputePipelineInit(&core, vkcv::ShaderStage::COMPUTE,
                                                                                    "shaders/flip.comp", flipPipeline);

    // particle rendering shaders
    vkcv::ShaderProgram particleShaderProgram{};
    compiler.compile(vkcv::ShaderStage::VERTEX, "shaders/shader.vert", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
        particleShaderProgram.addShader(shaderStage, path);
    });
    compiler.compile(vkcv::ShaderStage::FRAGMENT, "shaders/shader_water.frag", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
        particleShaderProgram.addShader(shaderStage, path);
    });

    // generating descriptorsets from shader reflection
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

    // initializing graphics pipeline
    vkcv::GraphicsPipelineConfig particlePipelineDefinition{
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

    vkcv::GraphicsPipelineHandle particlePipeline = core.createGraphicsPipeline(particlePipelineDefinition);

    vkcv::Buffer<glm::vec4> color = core.createBuffer<glm::vec4>(
            vkcv::BufferType::UNIFORM,
            1
    );

    vkcv::Buffer<glm::vec2> position = core.createBuffer<glm::vec2>(
            vkcv::BufferType::UNIFORM,
            1
    );

    // generating particles
    int numberParticles = 20000;
    std::vector<Particle> particles;
    for (int i = 0; i < numberParticles; i++) {
        const float lo = 0.6;
        const float hi = 0.9;
        const float vlo = 0;
        const float vhi = 70;
        float x = lo + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(hi-lo)));
        float y = lo + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(hi-lo)));
        float z = lo + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(hi-lo)));
        float vx = vlo + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(vhi-vlo)));
        float vy = vlo + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(vhi-vlo)));
        float vz = vlo + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(vhi-vlo)));
        glm::vec3 pos = glm::vec3(x,y,z);
        glm::vec3 vel = glm::vec3(vx,vy,vz);
        //glm::vec3 vel = glm::vec3(0.0,0.0,0.0);
        particles.push_back(Particle(pos, vel));
    }

    // creating and filling particle buffer
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
    
    core.writeDescriptorSet(pressureDescriptorSet, computeWrites);
	core.writeDescriptorSet(forceDescriptorSet, computeWrites);
    core.writeDescriptorSet(updateDataDescriptorSet, computeWrites);
	core.writeDescriptorSet(flipDescriptorSet, computeWrites);

    // error message if creation of one pipeline failed
    if (!particlePipeline || !pressurePipeline || !forcePipeline || !updateDataPipeline || !flipPipeline)
    {
        std::cout << "Error. Could not create at least one pipeline. Exiting." << std::endl;
        return EXIT_FAILURE;
    }

    const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

    const vkcv::Mesh renderMesh({vertexBufferBindings}, particleIndexBuffer.getVulkanHandle(),
                                particleIndexBuffer.getCount());
    vkcv::DescriptorSetUsage descriptorUsage(0, core.getDescriptorSet(descriptorSet).vulkanHandle);

    auto pos = glm::vec2(0.f);

    std::vector<vkcv::DrawcallInfo> drawcalls;
    drawcalls.push_back(vkcv::DrawcallInfo(renderMesh, {descriptorUsage}, numberParticles));

    auto start = std::chrono::system_clock::now();

    glm::vec4 colorData = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
    uint32_t camIndex0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
    uint32_t camIndex1 = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);

    cameraManager.getCamera(camIndex0).setNearFar(0.1, 30);
    cameraManager.getCamera(camIndex1).setNearFar(0.1, 30);

    cameraManager.setActiveCamera(1);

    cameraManager.getCamera(camIndex0).setPosition(glm::vec3(0, 0, -2.5));
    cameraManager.getCamera(camIndex1).setPosition(glm::vec3(0.0f, 0.0f, -2.5f));
    cameraManager.getCamera(camIndex1).setCenter(glm::vec3(0.0f, 0.0f, 0.0f));

	auto swapchainExtent = core.getSwapchain(window.getSwapchainHandle()).getExtent();
	
    vkcv::ImageHandle colorBuffer = core.createImage(
			colorFormat,
			swapchainExtent.width,
			swapchainExtent.height,
			1, false, true, true
	).getHandle();
    BloomAndFlares bloomAndFlares(&core, colorFormat, swapchainExtent.width, swapchainExtent.height);

    //tone mapping shader & pipeline
    vkcv::ComputePipelineHandle tonemappingPipe;
    vkcv::DescriptorSetHandle tonemappingDescriptor = PipelineInit::ComputePipelineInit(&core, vkcv::ShaderStage::COMPUTE,
                                                                                        "shaders/tonemapping.comp", tonemappingPipe);


    while (vkcv::Window::hasOpenWindow()) {
        vkcv::Window::pollEvents();

        uint32_t swapchainWidth, swapchainHeight;
        if (!core.beginFrame(swapchainWidth, swapchainHeight, windowHandle)) {
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

        glm::vec3 gravityDir = glm::rotate(glm::mat4(1.0), glm::radians(rotationx), glm::vec3(0.f,0.f,1.f)) * glm::vec4(0.f,1.f,0.f,0.f);
        gravityDir = glm::rotate(glm::mat4(1.0), glm::radians(rotationy), glm::vec3(0.f,1.f,0.f)) * glm::vec4(gravityDir,0.f);

        renderingMatrices.view = cameraManager.getActiveCamera().getView();
        renderingMatrices.view = glm::rotate(renderingMatrices.view, glm::radians(rotationx), glm::vec3(0.f, 0.f, 1.f));
        renderingMatrices.view = glm::rotate(renderingMatrices.view, glm::radians(rotationy), glm::vec3(0.f, 1.f, 0.f));
        renderingMatrices.projection = cameraManager.getActiveCamera().getProjection();

        // keybindings rotation
        if (glfwGetKey(window.getWindow(), GLFW_KEY_LEFT) == GLFW_PRESS)
            rotationx += deltatime * 50;
        if (glfwGetKey(window.getWindow(), GLFW_KEY_RIGHT) == GLFW_PRESS)
            rotationx -= deltatime * 50;
        
        if (glfwGetKey(window.getWindow(), GLFW_KEY_UP) == GLFW_PRESS)
            rotationy += deltatime * 50;
        if (glfwGetKey(window.getWindow(), GLFW_KEY_DOWN) == GLFW_PRESS)
            rotationy -= deltatime * 50;

        // keybindings params
        if (glfwGetKey(window.getWindow(), GLFW_KEY_T) == GLFW_PRESS)
            param_h += deltatime * 0.2;
        if (glfwGetKey(window.getWindow(), GLFW_KEY_G) == GLFW_PRESS)
            param_h -= deltatime * 0.2;

        if (glfwGetKey(window.getWindow(), GLFW_KEY_Y) == GLFW_PRESS)
            param_mass += deltatime * 0.2;
        if (glfwGetKey(window.getWindow(), GLFW_KEY_H) == GLFW_PRESS)
            param_mass -= deltatime * 0.2;

        if (glfwGetKey(window.getWindow(), GLFW_KEY_U) == GLFW_PRESS)
            param_gasConstant += deltatime * 1500.0;
        if (glfwGetKey(window.getWindow(), GLFW_KEY_J) == GLFW_PRESS)
            param_gasConstant -= deltatime * 1500.0;

        if (glfwGetKey(window.getWindow(), GLFW_KEY_I) == GLFW_PRESS)
            param_offset += deltatime * 400.0;
        if (glfwGetKey(window.getWindow(), GLFW_KEY_K) == GLFW_PRESS)
            param_offset -= deltatime * 400.0;

        if (glfwGetKey(window.getWindow(), GLFW_KEY_O) == GLFW_PRESS)
            param_viscosity = 50;
        if (glfwGetKey(window.getWindow(), GLFW_KEY_L) == GLFW_PRESS)
            param_viscosity = 1200;
        

        auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

        glm::vec4 pushData[3] = {
            glm::vec4(param_h,param_mass,param_gasConstant,param_offset),
            glm::vec4(param_gravity,param_viscosity,param_ABSORBTION,param_dt),
            glm::vec4(gravityDir.x,gravityDir.y,gravityDir.z,(float)numberParticles)
        };

        std::cout << "h: " << param_h << " | mass: " << param_mass << " | gasConstant: " << param_gasConstant << " | offset: " << param_offset << " | viscosity: " << param_viscosity << std::endl;

        vkcv::PushConstants pushConstantsCompute (sizeof(pushData));
        pushConstantsCompute.appendDrawcall(pushData);

        uint32_t computeDispatchCount[3] = {static_cast<uint32_t> (std::ceil(numberParticles/256.f)),1,1};

        // computing pressure pipeline
        core.recordComputeDispatchToCmdStream(cmdStream,
                                              pressurePipeline,
                                              computeDispatchCount,
                                              {vkcv::DescriptorSetUsage(0,core.getDescriptorSet(pressureDescriptorSet).vulkanHandle)},
											  pushConstantsCompute);

        core.recordBufferMemoryBarrier(cmdStream, particleBuffer1.getHandle());
		core.recordBufferMemoryBarrier(cmdStream, particleBuffer2.getHandle());

        // computing force pipeline
		core.recordComputeDispatchToCmdStream(cmdStream,
                                              forcePipeline,
                                              computeDispatchCount,
                                              {vkcv::DescriptorSetUsage(0,core.getDescriptorSet(forceDescriptorSet).vulkanHandle)},
                                              pushConstantsCompute);

		core.recordBufferMemoryBarrier(cmdStream, particleBuffer1.getHandle());
		core.recordBufferMemoryBarrier(cmdStream, particleBuffer2.getHandle());

        // computing update data pipeline
        core.recordComputeDispatchToCmdStream(cmdStream,
                                              updateDataPipeline,
                                              computeDispatchCount,
                                              { vkcv::DescriptorSetUsage(0,core.getDescriptorSet(updateDataDescriptorSet).vulkanHandle) },
                                              pushConstantsCompute);

        core.recordBufferMemoryBarrier(cmdStream, particleBuffer1.getHandle());
        core.recordBufferMemoryBarrier(cmdStream, particleBuffer2.getHandle());

        // computing flip pipeline
        core.recordComputeDispatchToCmdStream(cmdStream,
                                              flipPipeline,
                                              computeDispatchCount,
                                              { vkcv::DescriptorSetUsage(0,core.getDescriptorSet(flipDescriptorSet).vulkanHandle) },
                                              pushConstantsCompute);

        core.recordBufferMemoryBarrier(cmdStream, particleBuffer1.getHandle());
        core.recordBufferMemoryBarrier(cmdStream, particleBuffer2.getHandle());


        // bloomAndFlares & tonemapping
        vkcv::PushConstants pushConstantsDraw (sizeof(renderingMatrices));
        pushConstantsDraw.appendDrawcall(renderingMatrices);
        
        core.recordDrawcallsToCmdStream(
                cmdStream,
                particlePass,
                particlePipeline,
				pushConstantsDraw,
                {drawcalls},
                { colorBuffer },
                windowHandle);

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
        core.endFrame(windowHandle);
    }

    return 0;
}

#include <iostream>
#include <vkcv/Core.hpp>
#include <vkcv/Buffer.hpp>
#include <vkcv/Pass.hpp>
#include <vkcv/camera/CameraManager.hpp>
#include <random>
#include <ctime>
#include <vkcv/shader/GLSLCompiler.hpp>
#include <vkcv/effects/BloomAndFlaresEffect.hpp>
#include <vkcv/effects/GammaCorrectionEffect.hpp>
#include <vkcv/tone/ReinhardToneMapping.hpp>

#include "PipelineInit.hpp"
#include "Particle.hpp"

int main(int argc, const char **argv) {
    const std::string applicationName = "SPH";

    vkcv::Features features;
	features.requireExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // creating core object that will handle all vulkan objects
    vkcv::Core core = vkcv::Core::create(
        applicationName,
        VK_MAKE_VERSION(0, 0, 1),
        { vk::QueueFlagBits::eTransfer, vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute },
        features
    );

    vkcv::WindowHandle windowHandle = core.createWindow(applicationName, 1280, 720, true);
    vkcv::Window& window = core.getWindow(windowHandle);

    vkcv::camera::CameraManager cameraManager(window);

    auto particleIndexBuffer = vkcv::buffer<uint16_t>(core, vkcv::BufferType::INDEX, 3);
    uint16_t indices[3] = {0, 1, 2};
    particleIndexBuffer.fill(&indices[0], sizeof(indices));

    vk::Format colorFormat = vk::Format::eR16G16B16A16Sfloat;
    vkcv::PassHandle particlePass = vkcv::passFormat(core, colorFormat);

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

    if (!particlePass)
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

    vkcv::Buffer<glm::vec3> vertexBuffer = vkcv::buffer<glm::vec3>(
			core,
            vkcv::BufferType::VERTEX,
            3
    );
    const std::vector<vkcv::VertexAttachment> vertexAttachments = particleShaderProgram.getVertexAttachments();

    const std::vector<vkcv::VertexBufferBinding> vertexBufferBindings = {
            vkcv::vertexBufferBinding(vertexBuffer)
	};

    std::vector<vkcv::VertexBinding> bindings;
    for (size_t i = 0; i < vertexAttachments.size(); i++) {
        bindings.push_back(vkcv::createVertexBinding(i, {vertexAttachments[i]}));
    }

    const vkcv::VertexLayout particleLayout { bindings };

    // initializing graphics pipeline
    vkcv::GraphicsPipelineConfig particlePipelineDefinition (
            particleShaderProgram,
            particlePass,
            {particleLayout},
            {descriptorSetLayout}
	);
	
    particlePipelineDefinition.setBlendMode(vkcv::BlendMode::Additive);

    const std::vector<glm::vec3> vertices = {glm::vec3(-0.012, 0.012, 0),
                                             glm::vec3(0.012, 0.012, 0),
                                             glm::vec3(0, -0.012, 0)};

    vertexBuffer.fill(vertices);

    vkcv::GraphicsPipelineHandle particlePipeline = core.createGraphicsPipeline(particlePipelineDefinition);

    vkcv::Buffer<glm::vec4> color = vkcv::buffer<glm::vec4>(
			core,
            vkcv::BufferType::UNIFORM,
            1
    );

    vkcv::Buffer<glm::vec2> position = vkcv::buffer<glm::vec2>(
			core,
            vkcv::BufferType::UNIFORM,
            1
    );
	
	const float rand_max = static_cast<float>(RAND_MAX);

    // generating particles
    int numberParticles = 20000;
    std::vector<Particle> particles;
	particles.reserve(numberParticles);
	
    for (int i = 0; i < numberParticles; i++) {
        const float lo = 0.6;
        const float hi = 0.9;
        const float vlo = 0;
        const float vhi = 70;
        float x = lo + static_cast <float> (rand()) /( static_cast <float> (rand_max/(hi-lo)));
        float y = lo + static_cast <float> (rand()) /( static_cast <float> (rand_max/(hi-lo)));
        float z = lo + static_cast <float> (rand()) /( static_cast <float> (rand_max/(hi-lo)));
        float vx = vlo + static_cast <float> (rand()) /( static_cast <float> (rand_max/(vhi-vlo)));
        float vy = vlo + static_cast <float> (rand()) /( static_cast <float> (rand_max/(vhi-vlo)));
        float vz = vlo + static_cast <float> (rand()) /( static_cast <float> (rand_max/(vhi-vlo)));
        glm::vec3 pos = glm::vec3(x,y,z);
        glm::vec3 vel = glm::vec3(vx,vy,vz);
        particles.emplace_back(pos, vel);
    }

    // creating and filling particle buffer
    vkcv::Buffer<Particle> particleBuffer1 = vkcv::buffer<Particle>(
			core,
            vkcv::BufferType::STORAGE,
            numberParticles
    );

    vkcv::Buffer<Particle> particleBuffer2 = vkcv::buffer<Particle>(
			core,
			vkcv::BufferType::STORAGE,
			numberParticles
    );

    particleBuffer1.fill(particles);
	particleBuffer2.fill(particles);

    vkcv::DescriptorWrites setWrites;
    setWrites.writeUniformBuffer(0, color.getHandle()).writeUniformBuffer(1, position.getHandle());
    setWrites.writeStorageBuffer(2, particleBuffer1.getHandle()).writeStorageBuffer(3, particleBuffer2.getHandle());
    core.writeDescriptorSet(descriptorSet, setWrites);

    vkcv::DescriptorWrites computeWrites;
    computeWrites.writeStorageBuffer(
			0, particleBuffer1.getHandle()
	).writeStorageBuffer(
			1, particleBuffer2.getHandle()
	);
    
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

	vkcv::VertexData vertexData (vertexBufferBindings);
	vertexData.setIndexBuffer(particleIndexBuffer.getHandle());
	vertexData.setCount(particleIndexBuffer.getCount());

    auto pos = glm::vec2(0.f);
	
	vkcv::InstanceDrawcall drawcall (vertexData, numberParticles);
	drawcall.useDescriptorSet(0, descriptorSet);
	
    glm::vec4 colorData = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
    auto camHandle0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
    auto camHandle1 = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);

    cameraManager.getCamera(camHandle0).setNearFar(0.1f, 30.0f);
	cameraManager.getCamera(camHandle0).setPosition(glm::vec3(0.0f, 0.0f, -2.5f));
	
    cameraManager.setActiveCamera(camHandle1);
	
	cameraManager.getCamera(camHandle1).setNearFar(0.1f, 30.0f);
    cameraManager.getCamera(camHandle1).setPosition(glm::vec3(0.0f, 0.0f, -2.5f));
    cameraManager.getCamera(camHandle1).setCenter(glm::vec3(0.0f, 0.0f, 0.0f));

	const auto swapchainExtent = core.getSwapchainExtent(window.getSwapchain());
	
	vkcv::ImageConfig colorBufferConfig (
			swapchainExtent.width,
			swapchainExtent.height
	);
	
	colorBufferConfig.setSupportingStorage(true);
	colorBufferConfig.setSupportingColorAttachment(true);
	
    vkcv::ImageHandle colorBuffer = core.createImage(
			colorFormat,
			colorBufferConfig
	);
	
	vkcv::effects::BloomAndFlaresEffect bloomAndFlares (core);
	bloomAndFlares.setUpsamplingLimit(3);
	
	vkcv::tone::ReinhardToneMapping toneMapping (core);
	vkcv::effects::GammaCorrectionEffect gammaCorrection (core);
	
	core.run([&](const vkcv::WindowHandle &windowHandle, double t, double dt,
				 uint32_t swapchainWidth, uint32_t swapchainHeight) {
		if ((core.getImageWidth(colorBuffer) != swapchainWidth) ||
			(core.getImageHeight(colorBuffer) != swapchainHeight)) {
			colorBufferConfig.setWidth(swapchainWidth);
			colorBufferConfig.setHeight(swapchainHeight);
			
			colorBuffer = core.createImage(
					colorFormat,
					colorBufferConfig
			);
		}

        color.fill(&colorData);
        position.fill(&pos);

        cameraManager.update(dt);

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
            rotationx += dt * 50;
        if (glfwGetKey(window.getWindow(), GLFW_KEY_RIGHT) == GLFW_PRESS)
            rotationx -= dt * 50;
        
        if (glfwGetKey(window.getWindow(), GLFW_KEY_UP) == GLFW_PRESS)
            rotationy += dt * 50;
        if (glfwGetKey(window.getWindow(), GLFW_KEY_DOWN) == GLFW_PRESS)
            rotationy -= dt * 50;

        // keybindings params
        if (glfwGetKey(window.getWindow(), GLFW_KEY_T) == GLFW_PRESS)
            param_h += dt * 0.2;
        if (glfwGetKey(window.getWindow(), GLFW_KEY_G) == GLFW_PRESS)
            param_h -= dt * 0.2;

        if (glfwGetKey(window.getWindow(), GLFW_KEY_Y) == GLFW_PRESS)
            param_mass += dt * 0.2;
        if (glfwGetKey(window.getWindow(), GLFW_KEY_H) == GLFW_PRESS)
            param_mass -= dt * 0.2;

        if (glfwGetKey(window.getWindow(), GLFW_KEY_U) == GLFW_PRESS)
            param_gasConstant += dt * 1500.0;
        if (glfwGetKey(window.getWindow(), GLFW_KEY_J) == GLFW_PRESS)
            param_gasConstant -= dt * 1500.0;

        if (glfwGetKey(window.getWindow(), GLFW_KEY_I) == GLFW_PRESS)
            param_offset += dt * 400.0;
        if (glfwGetKey(window.getWindow(), GLFW_KEY_K) == GLFW_PRESS)
            param_offset -= dt * 400.0;

        if (glfwGetKey(window.getWindow(), GLFW_KEY_O) == GLFW_PRESS)
            param_viscosity = 50;
        if (glfwGetKey(window.getWindow(), GLFW_KEY_L) == GLFW_PRESS)
            param_viscosity = 1200;
        

        auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

        glm::vec4 pushData[3] = {
            glm::vec4(param_h,param_mass,param_gasConstant,param_offset),
            glm::vec4(param_gravity,param_viscosity,param_ABSORBTION,dt * 0.1),
            glm::vec4(gravityDir.x,gravityDir.y,gravityDir.z,(float)numberParticles)
        };

        std::cout << "h: " << param_h << " | mass: " << param_mass << " | gasConstant: " << param_gasConstant << " | offset: " << param_offset << " | viscosity: " << param_viscosity << std::endl;

        vkcv::PushConstants pushConstantsCompute (sizeof(pushData));
        pushConstantsCompute.appendDrawcall(pushData);

        const auto computeDispatchCount = vkcv::dispatchInvocations(numberParticles, 256);

        // computing pressure pipeline
        core.recordComputeDispatchToCmdStream(
				cmdStream,
				pressurePipeline,
				computeDispatchCount,
				{ vkcv::useDescriptorSet(0, pressureDescriptorSet) },
				pushConstantsCompute
		);

        core.recordBufferMemoryBarrier(cmdStream, particleBuffer1.getHandle());
		core.recordBufferMemoryBarrier(cmdStream, particleBuffer2.getHandle());

        // computing force pipeline
		core.recordComputeDispatchToCmdStream(
				cmdStream,
				forcePipeline,
				computeDispatchCount,
				{ vkcv::useDescriptorSet(0, forceDescriptorSet) },
				pushConstantsCompute
		);

		core.recordBufferMemoryBarrier(cmdStream, particleBuffer1.getHandle());
		core.recordBufferMemoryBarrier(cmdStream, particleBuffer2.getHandle());

        // computing update data pipeline
        core.recordComputeDispatchToCmdStream(
				cmdStream,
				updateDataPipeline,
				computeDispatchCount,
				{ vkcv::useDescriptorSet(0, updateDataDescriptorSet) },
				pushConstantsCompute
		);

        core.recordBufferMemoryBarrier(cmdStream, particleBuffer1.getHandle());
        core.recordBufferMemoryBarrier(cmdStream, particleBuffer2.getHandle());

        // computing flip pipeline
        core.recordComputeDispatchToCmdStream(
				cmdStream,
				flipPipeline,
				computeDispatchCount,
				{ vkcv::useDescriptorSet(0, flipDescriptorSet) },
				pushConstantsCompute
		);

        core.recordBufferMemoryBarrier(cmdStream, particleBuffer1.getHandle());
        core.recordBufferMemoryBarrier(cmdStream, particleBuffer2.getHandle());


        // bloomAndFlares & tonemapping
        vkcv::PushConstants pushConstantsDraw (sizeof(renderingMatrices));
        pushConstantsDraw.appendDrawcall(renderingMatrices);
        
        core.recordDrawcallsToCmdStream(
                cmdStream,
                particlePipeline,
				pushConstantsDraw,
                { drawcall },
                { colorBuffer },
                windowHandle
		);
	
		bloomAndFlares.recordEffect(cmdStream, colorBuffer, colorBuffer);
		toneMapping.recordToneMapping(cmdStream, colorBuffer, colorBuffer);
		gammaCorrection.recordEffect(cmdStream, colorBuffer, swapchainInput);

        core.prepareSwapchainImageForPresent(cmdStream);
        core.submitCommandStream(cmdStream);
    });

    return 0;
}

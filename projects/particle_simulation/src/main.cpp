#include <iostream>
#include <vkcv/Buffer.hpp>
#include <vkcv/Core.hpp>
#include <vkcv/Pass.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include "ParticleSystem.hpp"
#include <random>
#include <vkcv/shader/GLSLCompiler.hpp>
#include <vkcv/effects/BloomAndFlaresEffect.hpp>
#include <vkcv/effects/GammaCorrectionEffect.hpp>
#include <vkcv/tone/ReinhardToneMapping.hpp>

int main(int argc, const char **argv) {
    const std::string applicationName = "Particlesystem";

    uint32_t windowWidth = 800;
    uint32_t windowHeight = 600;

    vkcv::Features features;
	features.requireExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    features.requireExtensionFeature<vk::PhysicalDeviceDescriptorIndexingFeatures>(
			VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
			[](vk::PhysicalDeviceDescriptorIndexingFeatures& features) {
				features.setDescriptorBindingPartiallyBound(true);
				features.setDescriptorBindingVariableDescriptorCount(true);
			}
	);
	
    vkcv::Core core = vkcv::Core::create(
            applicationName,
            VK_MAKE_VERSION(0, 0, 1),
            {vk::QueueFlagBits::eTransfer, vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute},
			features
    );
	vkcv::WindowHandle windowHandle = core.createWindow(applicationName, windowWidth, windowHeight, true);
    vkcv::Window& window = core.getWindow(windowHandle);
	vkcv::camera::CameraManager cameraManager(window);

    auto particleIndexBuffer = vkcv::buffer<uint16_t>(core, vkcv::BufferType::INDEX, 3,
													  vkcv::BufferMemoryType::DEVICE_LOCAL);
    uint16_t indices[3] = {0, 1, 2};
    particleIndexBuffer.fill(&indices[0], sizeof(indices));

    vk::Format colorFormat = vk::Format::eR16G16B16A16Sfloat;
    vkcv::PassHandle particlePass = vkcv::passFormat(core, colorFormat);

    if (!particlePass)
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

    vkcv::DescriptorSetLayoutHandle computeDescriptorSetLayout = core.createDescriptorSetLayout(computeShaderProgram.getReflectedDescriptors().at(0));
    vkcv::DescriptorSetHandle computeDescriptorSet = core.createDescriptorSet(computeDescriptorSetLayout);

    const std::vector<vkcv::VertexAttachment> computeVertexAttachments = computeShaderProgram.getVertexAttachments();

    std::vector<vkcv::VertexBinding> computeBindings;
    for (size_t i = 0; i < computeVertexAttachments.size(); i++) {
        computeBindings.push_back(vkcv::createVertexBinding(i, { computeVertexAttachments[i] }));
    }
    const vkcv::VertexLayout computeLayout { computeBindings };

    vkcv::ShaderProgram particleShaderProgram{};
    compiler.compile(vkcv::ShaderStage::VERTEX, "shaders/shader.vert", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
        particleShaderProgram.addShader(shaderStage, path);
    });
    compiler.compile(vkcv::ShaderStage::FRAGMENT, shaderPathFragment, [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
        particleShaderProgram.addShader(shaderStage, path);
    });

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

    vkcv::ComputePipelineHandle computePipeline = core.createComputePipeline({
		computeShaderProgram, {computeDescriptorSetLayout}
	});

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

    glm::vec3 minVelocity = glm::vec3(-0.1f,-0.1f,-0.1f);
    glm::vec3 maxVelocity = glm::vec3(0.1f,0.1f,0.1f);
    glm::vec2 lifeTime = glm::vec2(-1.f,8.f);
    ParticleSystem particleSystem = ParticleSystem( 100000 , minVelocity, maxVelocity, lifeTime);

    vkcv::Buffer<Particle> particleBuffer = vkcv::buffer<Particle>(
			core,
            vkcv::BufferType::STORAGE,
            particleSystem.getParticles().size()
    );

    particleBuffer.fill(particleSystem.getParticles());

    vkcv::DescriptorWrites setWrites;
    setWrites.writeUniformBuffer(0, color.getHandle()).writeUniformBuffer(1, position.getHandle());
    setWrites.writeStorageBuffer(2, particleBuffer.getHandle());
    core.writeDescriptorSet(descriptorSet, setWrites);

    vkcv::DescriptorWrites computeWrites;
    computeWrites.writeStorageBuffer(0, particleBuffer.getHandle());
    core.writeDescriptorSet(computeDescriptorSet, computeWrites);

    if (!particlePipeline || !computePipeline)
    {
        std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
        return EXIT_FAILURE;
    }

    const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

	vkcv::VertexData vertexData (vertexBufferBindings);
	vertexData.setIndexBuffer(particleIndexBuffer.getHandle());
	vertexData.setCount(particleIndexBuffer.getCount());
	
    auto descriptorUsage = vkcv::useDescriptorSet(0, descriptorSet);

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
	
	vkcv::InstanceDrawcall drawcall (vertexData, particleSystem.getParticles().size());
	drawcall.useDescriptorSet(0, descriptorSet);
	
    glm::vec4 colorData = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
    auto camHandle0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
    auto camHandle1 = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);

    cameraManager.getCamera(camHandle0).setNearFar(0.1f, 30.0f);
    cameraManager.getCamera(camHandle1).setNearFar(0.1f, 30.0f);

    cameraManager.setActiveCamera(camHandle1);

    cameraManager.getCamera(camHandle0).setPosition(glm::vec3(0.0f, 0.0f, -2.0f));
    cameraManager.getCamera(camHandle1).setPosition(glm::vec3(0.0f, 0.0f, -2.0f));

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

    std::uniform_real_distribution<float> rdm = std::uniform_real_distribution<float>(0.95f, 1.05f);
    std::default_random_engine rdmEngine;
	
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
        
        renderingMatrices.view = cameraManager.getActiveCamera().getView();
        renderingMatrices.projection = cameraManager.getActiveCamera().getProjection();

        auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);
        float random = rdm(rdmEngine);
        glm::vec2 pushData = glm::vec2(dt, random);

        vkcv::PushConstants pushConstantsCompute = vkcv::pushConstants<glm::vec2>();
        pushConstantsCompute.appendDrawcall(pushData);
        
        core.recordComputeDispatchToCmdStream(
				cmdStream,
				computePipeline,
				vkcv::dispatchInvocations(particleSystem.getParticles().size(), 256),
				{vkcv::useDescriptorSet(0, computeDescriptorSet)},
				pushConstantsCompute
		);

        core.recordBufferMemoryBarrier(cmdStream, particleBuffer.getHandle());

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

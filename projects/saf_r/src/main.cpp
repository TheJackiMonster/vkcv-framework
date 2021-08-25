#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include <chrono>
#include <limits>
#include <cmath>
#include <vector>
#include <string.h>	// memcpy(3)
#include "safrScene.hpp"


int main(int argc, const char** argv) {
	const char* applicationName = "SAF_R";

	const int windowWidth = 800;
	const int windowHeight = 600;
	vkcv::Window window = vkcv::Window::create(
		applicationName,
		windowWidth,
		windowHeight,
		false
	);

	vkcv::Core core = vkcv::Core::create(
		window,
		applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		{ vk::QueueFlagBits::eTransfer,vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute },
		{},
		{ "VK_KHR_swapchain" }
	);

	//configuring the compute Shader
	vkcv::PassConfig computePassDefinition({});
	vkcv::PassHandle computePass = core.createPass(computePassDefinition);

	if (!computePass)
	{
		std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	std::string shaderPathCompute = "shaders/raytracing.comp";

	vkcv::ShaderProgram safrShaderProgram;
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
	

	compiler.compile(vkcv::ShaderStage::VERTEX, std::filesystem::path("shaders/shader.vert"),
		[&safrShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
			safrShaderProgram.addShader(shaderStage, path);
		});

	compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("shaders/shader.frag"),
		[&safrShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
			safrShaderProgram.addShader(shaderStage, path);
		});

	uint32_t setID = 0;
	std::vector<vkcv::DescriptorBinding> descriptorBindings = { safrShaderProgram.getReflectedDescriptors()[setID] };
	vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(descriptorBindings);

	//materials for the spheres
	std::vector<safrScene::Material> materials;
	safrScene::Material ivory(glm::vec3(0.6, 0.3, 0.1), glm::vec3(0.4, 0.4, 0.3), 50.);
	safrScene::Material red_rubber(glm::vec3(0.9, 0.1, 0.0), glm::vec3(0.3, 0.1, 0.1), 10.);
	safrScene::Material mirror(glm::vec3(0.0, 10.0, 0.8), glm::vec3(1.0, 1.0, 1.0), 1425.);
	materials.push_back(ivory);
	materials.push_back(red_rubber);
	materials.push_back(mirror);

	//spheres for the scene
	std::vector<safrScene::Sphere> spheres;
	spheres.push_back(safrScene::Sphere(glm::vec3(-3, 0, -16), 2, ivory));
	spheres.push_back(safrScene::Sphere(glm::vec3(-1.0, -1.5, -12), 2, mirror));
	spheres.push_back(safrScene::Sphere(glm::vec3(1.5, -0.5, -18), 3, red_rubber));
	spheres.push_back(safrScene::Sphere(glm::vec3(7, 5, -18), 4, mirror));

	//lights for the scene
	std::vector<safrScene::Light> lights;
	lights.push_back(safrScene::Light(glm::vec3(-20, 20, 20), 1.5));
	lights.push_back(safrScene::Light(glm::vec3(30, 50, -25), 1.8));
	lights.push_back(safrScene::Light(glm::vec3(30, 20, 30), 1.7));
	//create the raytracer image for rendering
	safrScene scene;
	safrScene::TextureData texData = scene.render(spheres, lights);

	vkcv::Image texture = core.createImage(vk::Format::eR8G8B8A8Unorm, texData.width, texData.height);
	texture.fill(texData.data.data());
	texture.generateMipChainImmediate();
	texture.switchLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

	vkcv::SamplerHandle sampler = core.createSampler(
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerMipmapMode::LINEAR,
		vkcv::SamplerAddressMode::REPEAT
	);

	//create Buffer for compute shader
	vkcv::Buffer<safrScene::Light> lightsBuffer = core.createBuffer<safrScene::Light>(
		vkcv::BufferType::STORAGE,
		lights.size()
		);
	lightsBuffer.fill(lights);

	vkcv::Buffer<safrScene::Sphere> sphereBuffer = core.createBuffer<safrScene::Sphere>(
		vkcv::BufferType::STORAGE,
		spheres.size()
		);
	sphereBuffer.fill(spheres);

	vkcv::Buffer<safrScene::Material> materialBuffer = core.createBuffer<safrScene::Material>(
		vkcv::BufferType::STORAGE,
		materials.size()
		);
	materialBuffer.fill(materials);


	vkcv::DescriptorWrites setWrites;
	setWrites.sampledImageWrites = { vkcv::SampledImageDescriptorWrite(0, texture.getHandle()) };
	setWrites.samplerWrites = { vkcv::SamplerDescriptorWrite(1, sampler) };
	core.writeDescriptorSet(descriptorSet, setWrites);

	vkcv::DescriptorWrites computeWrites;
	computeWrites.storageBufferWrites = { vkcv::BufferDescriptorWrite(0,lightsBuffer.getHandle()) };
	computeWrites.storageBufferWrites = { vkcv::BufferDescriptorWrite(1,materialBuffer.getHandle()) };
	computeWrites.storageBufferWrites = { vkcv::BufferDescriptorWrite(2,sphereBuffer.getHandle()) };
	core.writeDescriptorSet(computeDescriptorSet, computeWrites);

	const auto& context = core.getContext();

	auto safrIndexBuffer = core.createBuffer<uint16_t>(vkcv::BufferType::INDEX, 3, vkcv::BufferMemoryType::DEVICE_LOCAL);
	uint16_t indices[3] = { 0, 1, 2 };
	safrIndexBuffer.fill(&indices[0], sizeof(indices));

	// an example attachment for passes that output to the window
	const vkcv::AttachmentDescription present_color_attachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		core.getSwapchain().getFormat());

	vkcv::PassConfig safrPassDefinition({ present_color_attachment });
	vkcv::PassHandle safrPass = core.createPass(safrPassDefinition);

	if (!safrPass)
	{
		std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
		return EXIT_FAILURE;
	}


	const vkcv::PipelineConfig safrPipelineDefinition{
			safrShaderProgram,
			(uint32_t)windowWidth,
			(uint32_t)windowHeight,
			safrPass,
			{},
			{ core.getDescriptorSet(descriptorSet).layout },
			false
	};

	vkcv::PipelineHandle safrPipeline = core.createGraphicsPipeline(safrPipelineDefinition);
	vkcv::PipelineHandle computePipeline = core.createComputePipeline(computeShaderProgram, { core.getDescriptorSet(computeDescriptorSet).layout });

	if (!safrPipeline || !computePipeline)
	{
		std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	auto start = std::chrono::system_clock::now();

	const vkcv::Mesh renderMesh({}, safrIndexBuffer.getVulkanHandle(), 3);
	vkcv::DescriptorSetUsage descriptorUsage(0, core.getDescriptorSet(descriptorSet).vulkanHandle);
	vkcv::DrawcallInfo drawcall(renderMesh, { descriptorUsage }, 1);

	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

	vkcv::camera::CameraManager cameraManager(window);
	uint32_t camIndex0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	uint32_t camIndex1 = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);

	cameraManager.getCamera(camIndex0).setPosition(glm::vec3(0, 0, -2));
	cameraManager.getCamera(camIndex1).setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	cameraManager.getCamera(camIndex1).setCenter(glm::vec3(0.0f, 0.0f, -1.0f));

	while (window.isWindowOpen())
	{
		vkcv::Window::pollEvents();

		uint32_t swapchainWidth, swapchainHeight; // No resizing = No problem
		if (!core.beginFrame(swapchainWidth, swapchainHeight)) {
			continue;
		}


		auto end = std::chrono::system_clock::now();
		auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		start = end;

		cameraManager.update(0.000001 * static_cast<double>(deltatime.count()));
		glm::mat4 mvp = cameraManager.getActiveCamera().getMVP();
		glm::mat4 proj = cameraManager.getActiveCamera().getProjection();

		vkcv::PushConstants pushConstants(sizeof(glm::mat4) * 2);
		pushConstants.appendDrawcall(std::array<glm::mat4, 2>{ mvp, proj });

		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		vkcv::PushConstants pushConstantsCompute(0);
		//pushConstantsCompute.appendDrawcall(pushData);

		uint32_t computeDispatchCount[3] = { 2,1,1 };
		core.recordComputeDispatchToCmdStream(cmdStream,
			computePipeline,
			computeDispatchCount,
			{ vkcv::DescriptorSetUsage(0,core.getDescriptorSet(computeDescriptorSet).vulkanHandle) },
			pushConstantsCompute);

		core.recordBufferMemoryBarrier(cmdStream, lightsBuffer.getHandle());

		core.recordDrawcallsToCmdStream(
			cmdStream,
			safrPass,
			safrPipeline,
			pushConstants,
			{ drawcall },
			{ swapchainInput });

		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);

		core.endFrame();
	}
	return 0;
}

#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include <chrono>
#include <cmath>
#include <vector>
#include <cstring>
#include "safrScene.hpp"


void createQuadraticLightCluster(std::vector<safrScene::Light>& lights, int countPerDimension, float dimension, float height, float intensity) {
    float distance = dimension/countPerDimension;

    for(int x = 0; x <= countPerDimension; x++) {
        for (int z = 0; z <= countPerDimension; z++) {
            lights.push_back(safrScene::Light(glm::vec3(x * distance, height,  z * distance),
                                              float (intensity/countPerDimension) / 10.f) // Divide by 10, because intensity is busting O.o
                                              );
        }
    }

}

int main(int argc, const char** argv) {
	const char* applicationName = "SAF_R";

	//window creation
	const int windowWidth = 800;
	const int windowHeight = 600;

	vkcv::Core core = vkcv::Core::create(
		applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		{ vk::QueueFlagBits::eTransfer,vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute },
		{ "VK_KHR_swapchain" }
	);

	vkcv::WindowHandle windowHandle = core.createWindow(applicationName, windowWidth, windowHeight, true);

	//configuring the compute Shader
	vkcv::PassConfig computePassDefinition({});
	vkcv::PassHandle computePass = core.createPass(computePassDefinition);

	if (!computePass)
	{
		std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	std::string shaderPathCompute = "shaders/raytracing.comp";

	//creating the shader programs
	vkcv::ShaderProgram safrShaderProgram;
	vkcv::shader::GLSLCompiler compiler;
	vkcv::ShaderProgram computeShaderProgram{};

	compiler.compile(vkcv::ShaderStage::VERTEX, std::filesystem::path("shaders/shader.vert"),
		[&safrShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
			safrShaderProgram.addShader(shaderStage, path);
	});

	compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("shaders/shader.frag"),
		[&safrShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
			safrShaderProgram.addShader(shaderStage, path);
	});

	compiler.compile(vkcv::ShaderStage::COMPUTE, shaderPathCompute, [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		computeShaderProgram.addShader(shaderStage, path);
	});

	//create DescriptorSets (...) for every Shader
	const vkcv::DescriptorBindings& descriptorBindings = safrShaderProgram.getReflectedDescriptors().at(0);
	vkcv::DescriptorSetLayoutHandle descriptorSetLayout = core.createDescriptorSetLayout(descriptorBindings);
	vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(descriptorSetLayout);
	vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

	const vkcv::DescriptorBindings& computeDescriptorBindings = computeShaderProgram.getReflectedDescriptors().at(0);
	
	vkcv::DescriptorSetLayoutHandle computeDescriptorSetLayout = core.createDescriptorSetLayout(computeDescriptorBindings);
	vkcv::DescriptorSetHandle computeDescriptorSet = core.createDescriptorSet(computeDescriptorSetLayout);

	const std::vector<vkcv::VertexAttachment> computeVertexAttachments = computeShaderProgram.getVertexAttachments();

	std::vector<vkcv::VertexBinding> computeBindings;
	for (size_t i = 0; i < computeVertexAttachments.size(); i++) {
		computeBindings.push_back(vkcv::VertexBinding(i, { computeVertexAttachments[i] }));
	}
	const vkcv::VertexLayout computeLayout(computeBindings);
	
	/*
	* create the scene
	*/

	//materials for the spheres
	std::vector<safrScene::Material> materials;
	safrScene::Material ivory(glm::vec4(0.6, 0.3, 0.1, 0.0), glm::vec3(0.4, 0.4, 0.3), 50., 1.0);
	safrScene::Material red_rubber(glm::vec4(0.9, 0.1, 0.0, 0.0), glm::vec3(0.3, 0.1, 0.1), 10., 1.0);
	safrScene::Material mirror( glm::vec4(0.0, 10.0, 0.8, 0.0), glm::vec3(1.0, 1.0, 1.0), 1425., 1.0);
    safrScene::Material glass( glm::vec4(0.0, 10.0, 0.8, 0.0), glm::vec3(1.0, 1.0, 1.0), 1425., 1.5);

	materials.push_back(ivory);
	materials.push_back(red_rubber);
	materials.push_back(mirror);

	//spheres for the scene
	std::vector<safrScene::Sphere> spheres;
	spheres.push_back(safrScene::Sphere(glm::vec3(-3,    0,   -16), 2, ivory));
	// spheres.push_back(safrScene::Sphere(glm::vec3(-1.0, -1.5, 12), 2, mirror));
	spheres.push_back(safrScene::Sphere(glm::vec3(-1.0, -1.5, -12), 2, glass));
	spheres.push_back(safrScene::Sphere(glm::vec3(  1.5, -0.5, -18), 3, red_rubber));
	spheres.push_back(safrScene::Sphere(glm::vec3( 7,    5,   -18), 4, mirror));

	//lights for the scene
	std::vector<safrScene::Light> lights;
	/*
	lights.push_back(safrScene::Light(glm::vec3(-20, 20,  20), 1.5));
	lights.push_back(safrScene::Light(glm::vec3(30,  50, -25), 1.8));
	lights.push_back(safrScene::Light(glm::vec3(30,  20,  30), 1.7));
    */
    createQuadraticLightCluster(lights, 10, 2.5f, 20, 1.5f);


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

	vkcv::DescriptorWrites computeWrites;
	computeWrites.storageBufferWrites = { vkcv::BufferDescriptorWrite(0,lightsBuffer.getHandle()),
                                          vkcv::BufferDescriptorWrite(1,sphereBuffer.getHandle())};
    core.writeDescriptorSet(computeDescriptorSet, computeWrites);

	auto safrIndexBuffer = core.createBuffer<uint16_t>(vkcv::BufferType::INDEX, 3, vkcv::BufferMemoryType::DEVICE_LOCAL);
	uint16_t indices[3] = { 0, 1, 2 };
	safrIndexBuffer.fill(&indices[0], sizeof(indices));

	// an example attachment for passes that output to the window
	const vkcv::AttachmentDescription present_color_attachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		core.getSwapchain(windowHandle).getFormat());

	vkcv::PassConfig safrPassDefinition({ present_color_attachment });
	vkcv::PassHandle safrPass = core.createPass(safrPassDefinition);

	if (!safrPass)
	{
		std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	//create the render pipeline + compute pipeline
	const vkcv::GraphicsPipelineConfig safrPipelineDefinition{
			safrShaderProgram,
			UINT32_MAX,
			UINT32_MAX,
			safrPass,
			{},
			{ descriptorSetLayout },
			true
	};

	vkcv::GraphicsPipelineHandle safrPipeline = core.createGraphicsPipeline(safrPipelineDefinition);

	const vkcv::ComputePipelineConfig computePipelineConfig{
			computeShaderProgram,
			{computeDescriptorSetLayout}
	};

	vkcv::ComputePipelineHandle computePipeline = core.createComputePipeline(computePipelineConfig);

	if (!safrPipeline || !computePipeline)
	{
		std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	
	auto start = std::chrono::system_clock::now();

	const vkcv::Mesh renderMesh({}, safrIndexBuffer.getVulkanHandle(), 3);
	vkcv::DescriptorSetUsage descriptorUsage(0, descriptorSet);
	vkcv::DrawcallInfo drawcall(renderMesh, { descriptorUsage }, 1);

	//create the camera
	vkcv::camera::CameraManager cameraManager(core.getWindow(windowHandle));
	uint32_t camIndex0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	uint32_t camIndex1 = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);

	cameraManager.getCamera(camIndex0).setPosition(glm::vec3(0, 0, 2));
	cameraManager.getCamera(camIndex1).setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	cameraManager.getCamera(camIndex1).setCenter(glm::vec3(0.0f, 0.0f, -1.0f));

	float time = 0;
	
	while (vkcv::Window::hasOpenWindow())
	{
		vkcv::Window::pollEvents();

		uint32_t swapchainWidth, swapchainHeight; // No resizing = No problem
		if (!core.beginFrame(swapchainWidth, swapchainHeight, windowHandle)) {
			continue;
		}

		//configure timer
		auto end = std::chrono::system_clock::now();
		auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		start = end;
		
		time += 0.000001f * static_cast<float>(deltatime.count());
		
		//adjust light position
		/*
		639a53157e7d3936caf7c3e40379159cbcf4c89e
		lights[0].position.x += std::cos(time * 3.0f) * 2.5f;
		lights[1].position.z += std::cos(time * 2.5f) * 3.0f;
		lights[2].position.y += std::cos(time * 1.5f) * 4.0f;
		lightsBuffer.fill(lights);
		*/

		spheres[0].center.y += std::cos(time * 0.5f * 3.141f) * 0.25f;
		spheres[1].center.x += std::cos(time * 2.f) * 0.25f;
		spheres[1].center.z += std::cos(time * 2.f + 0.5f * 3.141f) * 0.25f;
        sphereBuffer.fill(spheres);

		//update camera
		cameraManager.update(0.000001 * static_cast<double>(deltatime.count()));
		glm::mat4 mvp = cameraManager.getActiveCamera().getMVP();
		glm::mat4 proj = cameraManager.getActiveCamera().getProjection();

		//create pushconstants for render
		vkcv::PushConstants pushConstants(sizeof(glm::mat4) * 2);
		pushConstants.appendDrawcall(std::array<glm::mat4, 2>{ mvp, proj });

		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		//configure the outImage for compute shader (render into the swapchain image)
        computeWrites.storageImageWrites = { vkcv::StorageImageDescriptorWrite(2, swapchainInput)};
        core.writeDescriptorSet(computeDescriptorSet, computeWrites);
        core.prepareImageForStorage (cmdStream, swapchainInput);

		//fill pushconstants for compute shader
        struct RaytracingPushConstantData {
            glm::mat4 viewToWorld;
            int32_t lightCount;
            int32_t sphereCount;
        };

        RaytracingPushConstantData raytracingPushData;
        raytracingPushData.lightCount   = lights.size();
        raytracingPushData.sphereCount  = spheres.size();
        raytracingPushData.viewToWorld  = glm::inverse(cameraManager.getActiveCamera().getView());

        vkcv::PushConstants pushConstantsCompute(sizeof(RaytracingPushConstantData));
        pushConstantsCompute.appendDrawcall(raytracingPushData);

		//dispatch compute shader
		uint32_t computeDispatchCount[3] = {static_cast<uint32_t> (std::ceil(swapchainWidth/16.f)),
                                            static_cast<uint32_t> (std::ceil(swapchainHeight/16.f)),
                                            1 }; // Anzahl workgroups
		core.recordComputeDispatchToCmdStream(cmdStream,
			computePipeline,
			computeDispatchCount,
			{ vkcv::DescriptorSetUsage(0, computeDescriptorSet) },
			pushConstantsCompute);

		core.recordBufferMemoryBarrier(cmdStream, lightsBuffer.getHandle());

		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);

		core.endFrame(windowHandle);
	}
	return 0;
}

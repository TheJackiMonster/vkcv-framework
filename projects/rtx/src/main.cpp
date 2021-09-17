#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include <vkcv/scene/Scene.hpp>
#include <vkcv/rtx/RTX.hpp>
#include <vkcv/rtx/RTXExtensions.hpp>

int main(int argc, const char** argv) {
	const char* applicationName = "RTX";

	uint32_t windowWidth = 800;
	uint32_t windowHeight = 600;

	// prepare raytracing extensions. IMPORTANT: configure compiler to build in 64 bit mode
	vkcv::rtx::RTXExtensions rtxExtensions;
	std::vector<const char*> raytracingInstanceExtensions = rtxExtensions.getInstanceExtensions();

	std::vector<const char*> instanceExtensions = {};   // add some more instance extensions, if needed
	instanceExtensions.insert(instanceExtensions.end(), raytracingInstanceExtensions.begin(), raytracingInstanceExtensions.end());  // merge together all instance extensions

	vkcv::Features features = rtxExtensions.getFeatures();  // all features required by the RTX device extensions
	features.requireExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	vkcv::Core core = vkcv::Core::create(
			applicationName,
			VK_MAKE_VERSION(0, 0, 1),
			{ vk::QueueFlagBits::eGraphics ,vk::QueueFlagBits::eCompute , vk::QueueFlagBits::eTransfer },
			features,
			instanceExtensions
	);

	vkcv::rtx::ASManager asManager(&core);

	vkcv::WindowHandle windowHandle = core.createWindow(applicationName, windowWidth, windowHeight, false);

	vkcv::camera::CameraManager cameraManager(core.getWindow(windowHandle));
	uint32_t camIndex0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	uint32_t camIndex1 = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);
	
	cameraManager.getCamera(camIndex0).setPosition(glm::vec3(-8, 1, -0.5));
	cameraManager.getCamera(camIndex0).setNearFar(0.1f, 30.0f);
	
	cameraManager.getCamera(camIndex1).setNearFar(0.1f, 30.0f);

	/*
	vkcv::scene::Scene scene = vkcv::scene::Scene::load(core, std::filesystem::path(
			argc > 1 ? argv[1] : "resources/Cube/cube.gltf"
	));
	
    
	vkcv::asset::Scene mesh;

	const char* path = argc > 1 ? argv[1] : "resources/cube/cube.gltf";
	int result = vkcv::asset::loadScene(path, mesh);

	if (result == 1) {
	    std::cout << "Mesh loading successful!" << std::endl;
	} else {
	    std::cerr << "Mesh loading failed: " << result << std::endl;
	    return 1;
	}

	assert(!mesh.vertexGroups.empty());
	*/
	// TODO: replace by bigger scene
	float cubeVertices[8*3] =
	{
		-1.f,-1.f,-1.f,
		1.f,-1.f,-1.f,
		1.f,1.f,-1.f,
		-1.f,1.f,-1.f,
		-1.f,-1.f,1.f,
		1.f,-1.f,1.f,
		1.f,1.f,1.f,
		-1.f,1.f,1.f
	};

	uint32_t cubeIndices[6 * 6] =
	{
		0, 1, 3, 3, 1, 2,
		1, 5, 2, 2, 5, 6,
		5, 4, 6, 6, 4, 7,
		4, 0, 7, 7, 0, 3,
		3, 2, 7, 7, 2, 6,
		4, 5, 0, 0, 5, 1
	};

	std::vector<float> vertices = {};
	for (size_t i = 0; i < std::size(cubeVertices)  ; i++) {
		vertices.push_back(cubeVertices[i]);
	}

	std::vector<uint32_t> indices = {};
	for (size_t i = 0; i < std::size(cubeIndices); i++) {
		indices.push_back(cubeIndices[i]);
	}


	
	//vkcv::ShaderProgram sceneShaderProgram;
	vkcv::shader::GLSLCompiler compiler;

	vkcv::ShaderProgram rayGenShaderProgram;
	compiler.compile(vkcv::ShaderStage::RAY_GEN, std::filesystem::path("resources/shaders/raytrace.rgen"),
		[&rayGenShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
			rayGenShaderProgram.addShader(shaderStage, path);
		});

	vkcv::ShaderProgram rayClosestHitShaderProgram;
	compiler.compile(vkcv::ShaderStage::RAY_CLOSEST_HIT, std::filesystem::path("resources/shaders/raytrace.rchit"),
		[&rayClosestHitShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
			rayClosestHitShaderProgram.addShader(shaderStage, path);
		});
	
	vkcv::ShaderProgram rayMissShaderProgram;
	compiler.compile(vkcv::ShaderStage::RAY_MISS, std::filesystem::path("resources/shaders/raytrace.rmiss"),
		[&rayMissShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
			rayMissShaderProgram.addShader(shaderStage, path);
		});

	std::vector<vkcv::DescriptorSetHandle> descriptorSetHandles;
	std::vector<vkcv::DescriptorSetLayoutHandle> descriptorSetLayoutHandles;

	vkcv::DescriptorSetLayoutHandle rayGenShaderDescriptorSetLayout = core.createDescriptorSetLayout(rayGenShaderProgram.getReflectedDescriptors().at(0));
	vkcv::DescriptorSetHandle rayGenShaderDescriptorSet = core.createDescriptorSet(rayGenShaderDescriptorSetLayout);//
	descriptorSetHandles.push_back(rayGenShaderDescriptorSet);
	descriptorSetLayoutHandles.push_back(rayGenShaderDescriptorSetLayout);

	vkcv::DescriptorSetLayoutHandle rayMissShaderDescriptorSetLayout = core.createDescriptorSetLayout(rayMissShaderProgram.getReflectedDescriptors().at(0));
	vkcv::DescriptorSetHandle rayMissShaderDescriptorSet = core.createDescriptorSet(rayMissShaderDescriptorSetLayout);
	descriptorSetHandles.push_back(rayMissShaderDescriptorSet);
	descriptorSetLayoutHandles.push_back(rayMissShaderDescriptorSetLayout);
	
	vkcv::DescriptorSetLayoutHandle rayClosestHitShaderDescriptorSetLayout = core.createDescriptorSetLayout(rayClosestHitShaderProgram.getReflectedDescriptors().at(0));
	vkcv::DescriptorSetHandle rayCHITShaderDescriptorSet = core.createDescriptorSet(rayClosestHitShaderDescriptorSetLayout);
	descriptorSetHandles.push_back(rayCHITShaderDescriptorSet);
	descriptorSetLayoutHandles.push_back(rayClosestHitShaderDescriptorSetLayout);

	

	// init RTXModule
	vkcv::rtx::RTXModule rtxModule(&core, &asManager, vertices, indices,descriptorSetHandles);

	struct RaytracingPushConstantData {
	    glm::vec4 camera_position;   // as origin for ray generation
	    glm::vec4 camera_right;      // for computing ray direction
	    glm::vec4 camera_up;         // for computing ray direction
	    glm::vec4 camera_forward;    // for computing ray direction
	    glm::uint frameCount;        // what is this? the actual frame?
	};

	uint32_t pushConstantSize = sizeof(RaytracingPushConstantData);

	rtxModule.createRTXPipeline(pushConstantSize, descriptorSetLayoutHandles, rayGenShaderProgram, rayMissShaderProgram, rayClosestHitShaderProgram);

	vk::Pipeline rtxPipeline = rtxModule.getPipeline();
	vk::PipelineLayout rtxPipelineLayout = rtxModule.getPipelineLayout();

	vkcv::ImageHandle depthBuffer = core.createImage(vk::Format::eD32Sfloat, windowWidth, windowHeight).getHandle();

	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

	vkcv::DescriptorWrites rtxWrites;

	auto start = std::chrono::system_clock::now();
	uint32_t frameCount = 0;
	while (vkcv::Window::hasOpenWindow()) {
        vkcv::Window::pollEvents();

		if(core.getWindow(windowHandle).getHeight() == 0 || core.getWindow(windowHandle).getWidth() == 0)
			continue;

		uint32_t swapchainWidth, swapchainHeight;
		if (!core.beginFrame(swapchainWidth, swapchainHeight,windowHandle)) {
			continue;
		}

		if ((swapchainWidth != windowWidth) || ((swapchainHeight != windowHeight))) {
			depthBuffer = core.createImage(vk::Format::eD32Sfloat, swapchainWidth, swapchainHeight).getHandle();

			windowWidth = swapchainWidth;
			windowHeight = swapchainHeight;
		}

		auto end = std::chrono::system_clock::now();
		auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

		start = end;
		cameraManager.update(0.000001 * static_cast<double>(deltatime.count()));

		const std::vector<vkcv::ImageHandle> renderTargets = { swapchainInput, depthBuffer };
		
		RaytracingPushConstantData raytracingPushData;
		raytracingPushData.camera_position = glm::vec4(cameraManager.getActiveCamera().getPosition(),0);
		raytracingPushData.camera_right = glm::vec4(glm::cross(cameraManager.getActiveCamera().getFront(), cameraManager.getActiveCamera().getUp()), 0);
		raytracingPushData.camera_up = glm::vec4(cameraManager.getActiveCamera().getUp(),0);
		raytracingPushData.camera_forward = glm::vec4(cameraManager.getActiveCamera().getFront(),0);
		raytracingPushData.frameCount = frameCount++;
		/*std::cout << "Camera position: [" << raytracingPushData.camera_position.x << "," <<
			raytracingPushData.camera_position.y << ","<< raytracingPushData.camera_position.z <<","<<
			raytracingPushData.camera_position.a << "]" <<std::endl;
		*/
		vkcv::PushConstants pushConstantsRTX(sizeof(RaytracingPushConstantData));
		pushConstantsRTX.appendDrawcall(raytracingPushData);

		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		
		rtxWrites.storageImageWrites = { vkcv::StorageImageDescriptorWrite(0, swapchainInput) };
		core.writeDescriptorSet(rayGenShaderDescriptorSet, rtxWrites);

		core.prepareImageForStorage(cmdStream, swapchainInput);

		core.recordRayGenerationToCmdStream(
			cmdStream,
			rtxPipeline,
			rtxPipelineLayout,
			rtxModule.getShaderBindingBuffer(),
			rtxModule.getShaderGroupBaseAlignment(),
			{	vkcv::DescriptorSetUsage(0, core.getDescriptorSet(rayGenShaderDescriptorSet).vulkanHandle),
				vkcv::DescriptorSetUsage(1, core.getDescriptorSet(rayMissShaderDescriptorSet).vulkanHandle),
				vkcv::DescriptorSetUsage(2, core.getDescriptorSet(rayCHITShaderDescriptorSet).vulkanHandle)
			},
			pushConstantsRTX,
			windowHandle);

		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
		core.endFrame(windowHandle);
	}

	return 0;
}

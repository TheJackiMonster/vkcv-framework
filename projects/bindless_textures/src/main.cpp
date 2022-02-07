#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>

int main(int argc, const char** argv) {
	const char* applicationName = "First Mesh";

	uint32_t windowWidth = 800;
	uint32_t windowHeight = 600;

	vkcv::Features features;
	features.requireExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    features.requireExtensionFeature<vk::PhysicalDeviceDescriptorIndexingFeatures>(
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, [](vk::PhysicalDeviceDescriptorIndexingFeatures &features) {
                features.setShaderInputAttachmentArrayDynamicIndexing(true);
                features.setShaderUniformTexelBufferArrayDynamicIndexing(true);
                features.setShaderStorageTexelBufferArrayDynamicIndexing(true);
                features.setShaderUniformBufferArrayNonUniformIndexing(true);
                features.setShaderSampledImageArrayNonUniformIndexing(true);
                features.setShaderStorageBufferArrayNonUniformIndexing(true);
                features.setShaderStorageImageArrayNonUniformIndexing(true);
                features.setShaderInputAttachmentArrayNonUniformIndexing(true);
                features.setShaderUniformTexelBufferArrayNonUniformIndexing(true);
                features.setShaderStorageTexelBufferArrayNonUniformIndexing(true);

                features.setDescriptorBindingUniformBufferUpdateAfterBind(true);
                features.setDescriptorBindingSampledImageUpdateAfterBind(true);
                features.setDescriptorBindingStorageImageUpdateAfterBind(true);
                features.setDescriptorBindingStorageBufferUpdateAfterBind(true);
                features.setDescriptorBindingUniformTexelBufferUpdateAfterBind(true);
                features.setDescriptorBindingStorageTexelBufferUpdateAfterBind(true);

                features.setDescriptorBindingUpdateUnusedWhilePending(true);
                features.setDescriptorBindingPartiallyBound(true);
                features.setDescriptorBindingVariableDescriptorCount(true);
                features.setRuntimeDescriptorArray(true);
            }
	);

	vkcv::Core core = vkcv::Core::create(
		applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		{ vk::QueueFlagBits::eGraphics ,vk::QueueFlagBits::eCompute , vk::QueueFlagBits::eTransfer },
		features
	);

	vkcv::WindowHandle windowHandle = core.createWindow(applicationName, windowWidth,windowHeight,false);

	vkcv::asset::Scene mesh;

	// TEST DATA
	std::vector<vkcv::Image> texturesArray;
    const std::string grassPaths[5] = { "resources/cube/Grass001_1K_AmbientOcclusion.jpg",
                                        "resources/cube/Grass001_1K_Color.jpg",
                                        "resources/cube/Grass001_1K_Displacement.jpg",
                                        "resources/cube/Grass001_1K_Normal.jpg",
                                        "resources/cube/Grass001_1K_Roughness.jpg" };
    for(uint32_t i = 0; i < 5; i++)
    {
        std::filesystem::path grassPath(grassPaths[i]);
        vkcv::asset::Texture grassTexture = vkcv::asset::loadTexture(grassPath);

        vkcv::Image texture = core.createImage(vk::Format::eR8G8B8A8Srgb, grassTexture.width, grassTexture.height);
        texture.fill(grassTexture.data.data());
        texture.generateMipChainImmediate();
        texture.switchLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

        texturesArray.push_back(texture);
    }

	const char* path = argc > 1 ? argv[1] : "resources/cube/cube.gltf";
	int result = vkcv::asset::loadScene(path, mesh);

	if (result == 1) {
		std::cout << "Mesh loading successful!" << std::endl;
	} else {
		std::cerr << "Mesh loading failed: " << result << std::endl;
		return 1;
	}

	assert(!mesh.vertexGroups.empty());
	auto vertexBuffer = core.createBuffer<uint8_t>(
			vkcv::BufferType::VERTEX,
			mesh.vertexGroups[0].vertexBuffer.data.size(),
			vkcv::BufferMemoryType::DEVICE_LOCAL
	);
	
	vertexBuffer.fill(mesh.vertexGroups[0].vertexBuffer.data);

	auto indexBuffer = core.createBuffer<uint8_t>(
			vkcv::BufferType::INDEX,
			mesh.vertexGroups[0].indexBuffer.data.size(),
			vkcv::BufferMemoryType::DEVICE_LOCAL
	);
	
	indexBuffer.fill(mesh.vertexGroups[0].indexBuffer.data);

	// an example attachment for passes that output to the window
	const vkcv::AttachmentDescription present_color_attachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		core.getSwapchain(windowHandle).getFormat()
	);
	
	const vkcv::AttachmentDescription depth_attachment(
			vkcv::AttachmentOperation::STORE,
			vkcv::AttachmentOperation::CLEAR,
			vk::Format::eD32Sfloat
	);

	vkcv::PassConfig firstMeshPassDefinition({ present_color_attachment, depth_attachment });
	vkcv::PassHandle firstMeshPass = core.createPass(firstMeshPassDefinition);

	if (!firstMeshPass) {
		std::cerr << "Error. Could not create renderpass. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	vkcv::ShaderProgram firstMeshProgram;
	vkcv::shader::GLSLCompiler compiler;
	
	compiler.compileProgram(firstMeshProgram, {
		{ vkcv::ShaderStage::VERTEX, "resources/shaders/shader.vert" },
		{ vkcv::ShaderStage::FRAGMENT, "resources/shaders/shader.frag" }
	}, nullptr);
 
	auto& attributes = mesh.vertexGroups[0].vertexBuffer.attributes;
	
	std::sort(attributes.begin(), attributes.end(), [](const vkcv::asset::VertexAttribute& x, const vkcv::asset::VertexAttribute& y) {
		return static_cast<uint32_t>(x.type) < static_cast<uint32_t>(y.type);
	});

    const std::vector<vkcv::VertexAttachment> vertexAttachments = firstMeshProgram.getVertexAttachments();
	std::vector<vkcv::VertexBinding> bindings;
	for (size_t i = 0; i < vertexAttachments.size(); i++) {
		bindings.push_back(vkcv::VertexBinding(i, { vertexAttachments[i] }));
	}
	
	const vkcv::VertexLayout firstMeshLayout (bindings);

	const std::unordered_map<uint32_t, vkcv::DescriptorBinding> &descriptorBindings = firstMeshProgram.getReflectedDescriptors().at(0);

    std::unordered_map<uint32_t, vkcv::DescriptorBinding> adjustedBindings = descriptorBindings;
    adjustedBindings[1].descriptorCount = 6;

    vkcv::DescriptorSetLayoutHandle descriptorSetLayout = core.createDescriptorSetLayout(adjustedBindings);
	vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(descriptorSetLayout);

	const vkcv::GraphicsPipelineConfig firstMeshPipelineConfig {
        firstMeshProgram,
        UINT32_MAX,
        UINT32_MAX,
        firstMeshPass,
        {firstMeshLayout},
		{ descriptorSetLayout },
		true
	};
	vkcv::GraphicsPipelineHandle firstMeshPipeline = core.createGraphicsPipeline(firstMeshPipelineConfig);
	
	if (!firstMeshPipeline) {
		std::cerr << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	
	if (mesh.textures.empty()) {
		std::cerr << "Error. No textures found. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	
	vkcv::asset::Texture &tex = mesh.textures[0];
	vkcv::Image texture = core.createImage(vk::Format::eR8G8B8A8Srgb, tex.w, tex.h);
	texture.fill(tex.data.data());
	texture.generateMipChainImmediate();
	texture.switchLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	texturesArray.push_back(texture);

	vkcv::SamplerHandle sampler = core.createSampler(
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerMipmapMode::LINEAR,
		vkcv::SamplerAddressMode::REPEAT
	);

	const std::vector<vkcv::VertexBufferBinding> vertexBufferBindings = {
		vkcv::VertexBufferBinding(static_cast<vk::DeviceSize>(attributes[0].offset), vertexBuffer.getVulkanHandle()),
		vkcv::VertexBufferBinding(static_cast<vk::DeviceSize>(attributes[1].offset), vertexBuffer.getVulkanHandle()),
		vkcv::VertexBufferBinding(static_cast<vk::DeviceSize>(attributes[2].offset), vertexBuffer.getVulkanHandle()) };

	vkcv::DescriptorWrites setWrites;
	std::vector<vkcv::SampledImageDescriptorWrite> texturesArrayWrites;
	for(uint32_t i = 0; i < 6; i++)
	{
	    texturesArrayWrites.push_back(vkcv::SampledImageDescriptorWrite(1,
                                                                        texturesArray[i].getHandle(),
                                                                        0,
                                                                        false,
                                                                        i));
	}

	setWrites.sampledImageWrites	= texturesArrayWrites;
	setWrites.samplerWrites			= { vkcv::SamplerDescriptorWrite(0, sampler) };

	core.writeDescriptorSet(descriptorSet, setWrites);

	vkcv::ImageHandle depthBuffer = core.createImage(vk::Format::eD32Sfloat, windowWidth, windowHeight, 1, false).getHandle();

	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

	const vkcv::Mesh renderMesh(vertexBufferBindings, indexBuffer.getVulkanHandle(), mesh.vertexGroups[0].numIndices);

	vkcv::DescriptorSetUsage    descriptorUsage(0, descriptorSet);
	vkcv::DrawcallInfo          drawcall(renderMesh, { descriptorUsage },1);

    vkcv::camera::CameraManager cameraManager(core.getWindow(windowHandle));
    uint32_t camIndex0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);
	
	cameraManager.getCamera(camIndex0).setPosition(glm::vec3(0, 0, -3));

    auto start = std::chrono::system_clock::now();
    
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
        glm::mat4 mvp = cameraManager.getActiveCamera().getMVP();

		vkcv::PushConstants pushConstants (sizeof(glm::mat4));
		pushConstants.appendDrawcall(mvp);

		const std::vector<vkcv::ImageHandle> renderTargets = { swapchainInput, depthBuffer };
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		core.recordDrawcallsToCmdStream(
			cmdStream,
			firstMeshPass,
			firstMeshPipeline,
			pushConstants,
			{ drawcall },
			renderTargets,
			windowHandle);
		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
		core.endFrame(windowHandle);
	}
	
	return 0;
}

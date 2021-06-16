#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include <vkcv/asset/asset_loader.hpp>

glm::mat4 arrayTo4x4Matrix(std::array<float,16> array){
    glm::mat4 matrix;
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            matrix[i][j] = array[j * 4 + i];
        }
    }
    return matrix;
}

int main(int argc, const char** argv) {
	const char* applicationName = "First Scene";

	uint32_t windowWidth = 800;
	uint32_t windowHeight = 600;

	vkcv::Window window = vkcv::Window::create(
		applicationName,
		windowWidth,
		windowHeight,
		true
	);

	vkcv::CameraManager cameraManager(window, static_cast<float>(window.getWidth()), static_cast<float>(window.getHeight()));

	window.initEvents();

	vkcv::Core core = vkcv::Core::create(
		window,
		applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		{ vk::QueueFlagBits::eGraphics ,vk::QueueFlagBits::eCompute , vk::QueueFlagBits::eTransfer },
		{},
		{ "VK_KHR_swapchain" }
	);

	vkcv::asset::Scene scene;

	const char* path = argc > 1 ? argv[1] : "resources/Cutlery/cutlerySzene.gltf";
	int result = vkcv::asset::loadScene(path, scene);

	if (result == 1) {
		std::cout << "Mesh loading successful!" << std::endl;
	}
	else {
		std::cout << "Mesh loading failed: " << result << std::endl;
		return 1;
	}

	assert(!scene.vertexGroups.empty());
	std::vector<uint8_t> vBuffers;
    std::vector<uint8_t> iBuffers;
	//vBuffers.reserve(scene.vertexGroups.size());
    //iBuffers.reserve(scene.vertexGroups.size());

    std::vector<vkcv::VertexBufferBinding> vBufferBindings;
    std::vector<std::vector<vkcv::VertexBufferBinding>> vertexBufferBindings;
    std::vector<vkcv::VertexAttribute> vAttributes;

    for (int i = 0; i < scene.vertexGroups.size(); i++){

        /*auto vertexBuffer = core.createBuffer<uint8_t>(
                vkcv::BufferType::VERTEX,
                scene.vertexGroups[i].vertexBuffer.data.size(),
                vkcv::BufferMemoryType::DEVICE_LOCAL
        );
        vertexBuffer.fill(scene.vertexGroups[i].vertexBuffer.data);*/
        vBuffers.insert(vBuffers.end(), scene.vertexGroups[i].vertexBuffer.data.begin(),scene.vertexGroups[i].vertexBuffer.data.end());

        /*auto indexBuffer = core.createBuffer<uint8_t>(
                vkcv::BufferType::INDEX,
                scene.vertexGroups[i].indexBuffer.data.size(),
                vkcv::BufferMemoryType::DEVICE_LOCAL
        );
        indexBuffer.fill(scene.vertexGroups[i].indexBuffer.data);*/
        iBuffers.insert(iBuffers.end(), scene.vertexGroups[i].indexBuffer.data.begin(),scene.vertexGroups[i].indexBuffer.data.end());

        auto& attributes = scene.vertexGroups[i].vertexBuffer.attributes;

        std::sort(attributes.begin(), attributes.end(), [](const vkcv::VertexAttribute& x, const vkcv::VertexAttribute& y) {
            return static_cast<uint32_t>(x.type) < static_cast<uint32_t>(y.type);
        });
    }

    auto vertexBuffer = core.createBuffer<uint8_t>(
            vkcv::BufferType::VERTEX,
            vBuffers.size(),
            vkcv::BufferMemoryType::DEVICE_LOCAL
    );
    vertexBuffer.fill(vBuffers);

    auto indexBuffer = core.createBuffer<uint8_t>(
            vkcv::BufferType::INDEX,
            iBuffers.size(),
            vkcv::BufferMemoryType::DEVICE_LOCAL
    );
    indexBuffer.fill(iBuffers);

    for (int m = 0; m < scene.vertexGroups.size(); m++){
        for (int k = 0; k < scene.vertexGroups[m].vertexBuffer.attributes.size(); k++){
            vAttributes.push_back(scene.vertexGroups[m].vertexBuffer.attributes[k]);
            vBufferBindings.push_back(vkcv::VertexBufferBinding(scene.vertexGroups[m].vertexBuffer.attributes[k].offset, vertexBuffer.getVulkanHandle()));
        }
        vertexBufferBindings.push_back(vBufferBindings);
        vBufferBindings.clear();
    }

	// an example attachment for passes that output to the window
	const vkcv::AttachmentDescription present_color_attachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		core.getSwapchainImageFormat()
	);
	
	const vkcv::AttachmentDescription depth_attachment(
			vkcv::AttachmentOperation::STORE,
			vkcv::AttachmentOperation::CLEAR,
			vk::Format::eD32Sfloat
	);

	vkcv::PassConfig scenePassDefinition({ present_color_attachment, depth_attachment });
	vkcv::PassHandle scenePass = core.createPass(scenePassDefinition);

	if (!scenePass) {
		std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	vkcv::ShaderProgram sceneShaderProgram{};
	sceneShaderProgram.addShader(vkcv::ShaderStage::VERTEX, std::filesystem::path("resources/shaders/vert.spv"));
    sceneShaderProgram.addShader(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("resources/shaders/frag.spv"));
    sceneShaderProgram.reflectShader(vkcv::ShaderStage::VERTEX);
    sceneShaderProgram.reflectShader(vkcv::ShaderStage::FRAGMENT);

	uint32_t setID = 0;
	std::vector<vkcv::DescriptorBinding> descriptorBindings = { sceneShaderProgram.getReflectedDescriptors()[setID] };
	vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(descriptorBindings);

	const vkcv::PipelineConfig scenePipelineDefinition(
		sceneShaderProgram,
        UINT32_MAX,
        UINT32_MAX,
		scenePass,
		vAttributes,
		{ core.getDescriptorSet(descriptorSet).layout },
		true);
	vkcv::PipelineHandle scenePipeline = core.createGraphicsPipeline(scenePipelineDefinition);
	
	if (!scenePipeline) {
		std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	
	// FIXME There should be a test here to make sure there is at least 1
	// texture in the scene.
	vkcv::asset::Texture &tex = scene.textures[0];
	vkcv::Image texture = core.createImage(vk::Format::eR8G8B8A8Srgb, tex.w, tex.h);
	texture.fill(tex.data.data());

	vkcv::SamplerHandle sampler = core.createSampler(
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerMipmapMode::LINEAR,
		vkcv::SamplerAddressMode::REPEAT
	);

	vkcv::DescriptorWrites setWrites;
	setWrites.sampledImageWrites    = { vkcv::SampledImageDescriptorWrite(0, texture.getHandle()) };
	setWrites.samplerWrites         = { vkcv::SamplerDescriptorWrite(1, sampler) };
	core.writeDescriptorSet(descriptorSet, setWrites);

	vkcv::ImageHandle depthBuffer = core.createImage(vk::Format::eD32Sfloat, windowWidth, windowHeight).getHandle();

	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

    vkcv::DescriptorSetUsage    descriptorUsage(0, core.getDescriptorSet(descriptorSet).vulkanHandle);
    std::vector<vkcv::DrawcallInfo> drawcalls;

	for(int l = 0; l < scene.vertexGroups.size(); l++){
        vkcv::Mesh renderMesh(vertexBufferBindings[l], indexBuffer.getVulkanHandle(), scene.vertexGroups[l].numIndices);
	    drawcalls.push_back(vkcv::DrawcallInfo(renderMesh, {descriptorUsage}));
	}

	std::vector<glm::mat4> modelMatrices;
	modelMatrices.clear();
	for(int m = 0; m < scene.meshes.size(); m++){
	    modelMatrices.push_back(arrayTo4x4Matrix(scene.meshes[m].modelMatrix));
	}
    std::vector<glm::mat4> mvp;

	auto start = std::chrono::system_clock::now();
	while (window.isWindowOpen()) {
        vkcv::Window::pollEvents();
		
		if(window.getHeight() == 0 || window.getWidth() == 0)
			continue;
		
		uint32_t swapchainWidth, swapchainHeight;
		if (!core.beginFrame(swapchainWidth, swapchainHeight)) {
			continue;
		}
		
		if ((swapchainWidth != windowWidth) || ((swapchainHeight != windowHeight))) {
			depthBuffer = core.createImage(vk::Format::eD32Sfloat, swapchainWidth, swapchainHeight).getHandle();
			
			windowWidth = swapchainWidth;
			windowHeight = swapchainHeight;
		}
  
		auto end = std::chrono::system_clock::now();
		auto deltatime = end - start;
		start = end;
		cameraManager.getCamera().updateView(std::chrono::duration<double>(deltatime).count());
		const glm::mat4 vp = cameraManager.getCamera().getProjection() * cameraManager.getCamera().getView();

		mvp.clear();
        for (const auto& m : modelMatrices) {
            mvp.push_back(vp * m);
        }

		vkcv::PushConstantData pushConstantData((void*)mvp.data(), sizeof(glm::mat4));

		const std::vector<vkcv::ImageHandle> renderTargets = { swapchainInput, depthBuffer };
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		core.recordDrawcallsToCmdStream(
			cmdStream,
			scenePass,
			scenePipeline,
			pushConstantData,
			drawcalls,
			renderTargets);
		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
		core.endFrame();
	}
	
	return 0;
}

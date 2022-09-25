#include <iostream>
#include <vkcv/Buffer.hpp>
#include <vkcv/Core.hpp>
#include <vkcv/Image.hpp>
#include <vkcv/Pass.hpp>
#include <vkcv/Sampler.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>

int main(int argc, const char** argv) {
	const std::string applicationName = "Bindless Textures";

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

	vkcv::WindowHandle windowHandle = core.createWindow(applicationName, 800, 600, true);
	vkcv::Window& window = core.getWindow(windowHandle);

	vkcv::asset::Scene mesh;

	// TEST DATA
	std::vector<vkcv::Image> texturesArray;
    const std::string grassPaths[5] = { "resources/cube/Grass001_1K_AmbientOcclusion.jpg",
                                        "resources/cube/Grass001_1K_Color.jpg",
                                        "resources/cube/Grass001_1K_Displacement.jpg",
                                        "resources/cube/Grass001_1K_Normal.jpg",
                                        "resources/cube/Grass001_1K_Roughness.jpg" };
	
    for (const auto &path : grassPaths) {
        std::filesystem::path grassPath(path);
        vkcv::asset::Texture grassTexture = vkcv::asset::loadTexture(grassPath);

        vkcv::Image texture = vkcv::image(
				core,
				vk::Format::eR8G8B8A8Srgb,
				grassTexture.width,
				grassTexture.height
		);
		
        texture.fill(grassTexture.data.data());
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
	auto vertexBuffer = vkcv::buffer<uint8_t>(
			core,
			vkcv::BufferType::VERTEX,
			mesh.vertexGroups[0].vertexBuffer.data.size(),
			vkcv::BufferMemoryType::DEVICE_LOCAL
	);
	
	vertexBuffer.fill(mesh.vertexGroups[0].vertexBuffer.data);

	auto indexBuffer = vkcv::buffer<uint8_t>(
			core,
			vkcv::BufferType::INDEX,
			mesh.vertexGroups[0].indexBuffer.data.size(),
			vkcv::BufferMemoryType::DEVICE_LOCAL
	);
	
	indexBuffer.fill(mesh.vertexGroups[0].indexBuffer.data);
	
	vkcv::PassHandle firstMeshPass = vkcv::passSwapchain(
			core,
			window.getSwapchain(),
			{ vk::Format::eUndefined, vk::Format::eD32Sfloat }
	);

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
	
	const auto vertexBufferBindings = vkcv::asset::loadVertexBufferBindings(
			mesh.vertexGroups[0].vertexBuffer.attributes,
			vertexBuffer.getHandle(),
			{
					vkcv::asset::PrimitiveType::POSITION,
					vkcv::asset::PrimitiveType::NORMAL,
					vkcv::asset::PrimitiveType::TEXCOORD_0
			}
	);
	
	std::vector<vkcv::VertexBinding> bindings = vkcv::createVertexBindings(
			firstMeshProgram.getVertexAttachments()
	);
	
	const vkcv::VertexLayout firstMeshLayout { bindings };
	const std::unordered_map<uint32_t, vkcv::DescriptorBinding> &descriptorBindings = firstMeshProgram.getReflectedDescriptors().at(0);

    std::unordered_map<uint32_t, vkcv::DescriptorBinding> adjustedBindings = descriptorBindings;
    adjustedBindings[1].descriptorCount = 6;

    vkcv::DescriptorSetLayoutHandle descriptorSetLayout = core.createDescriptorSetLayout(adjustedBindings);
	vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(descriptorSetLayout);

	vkcv::GraphicsPipelineHandle firstMeshPipeline = core.createGraphicsPipeline(
			vkcv::GraphicsPipelineConfig(
					firstMeshProgram,
					firstMeshPass,
					{ firstMeshLayout },
					{ descriptorSetLayout }
			)
	);
	
	if (!firstMeshPipeline) {
		std::cerr << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	
	if (mesh.textures.empty()) {
		std::cerr << "Error. No textures found. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	
	{
		vkcv::asset::Texture &tex = mesh.textures[0];
		vkcv::Image texture = vkcv::image(core, vk::Format::eR8G8B8A8Srgb, tex.w, tex.h);
		texture.fill(tex.data.data());
		texturesArray.push_back(texture);
	}
	
	auto downsampleStream = core.createCommandStream(vkcv::QueueType::Graphics);
	
	for (auto& texture : texturesArray) {
		texture.recordMipChainGeneration(downsampleStream, core.getDownsampler());
	}
	
	core.submitCommandStream(downsampleStream, false);

	vkcv::SamplerHandle sampler = vkcv::samplerLinear(core);
	vkcv::DescriptorWrites setWrites;
	
	for(uint32_t i = 0; i < 6; i++)
	{
		
		setWrites.writeSampledImage(
				1,
				texturesArray[i].getHandle(),
				0,
				false,
				i
		);
	}

	setWrites.writeSampler(0, sampler);

	core.writeDescriptorSet(descriptorSet, setWrites);

	vkcv::ImageHandle depthBuffer;

	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();
	
	vkcv::VertexData vertexData (vertexBufferBindings);
	vertexData.setIndexBuffer(indexBuffer.getHandle());
	vertexData.setCount(mesh.vertexGroups[0].numIndices);
	
	vkcv::InstanceDrawcall drawcall (vertexData);
	drawcall.useDescriptorSet(0, descriptorSet);

    vkcv::camera::CameraManager cameraManager(window);
    auto camHandle0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);
	
	cameraManager.getCamera(camHandle0).setPosition(glm::vec3(0, 0, -3));
	
	core.run([&](const vkcv::WindowHandle &windowHandle, double t, double dt,
				 uint32_t swapchainWidth, uint32_t swapchainHeight) {
		if ((!depthBuffer) ||
			(swapchainWidth != core.getImageWidth(depthBuffer)) ||
			(swapchainHeight != core.getImageHeight(depthBuffer))) {
			depthBuffer = core.createImage(
					vk::Format::eD32Sfloat,
					swapchainWidth,
					swapchainHeight
			);
		}

		cameraManager.update(dt);
        glm::mat4 mvp = cameraManager.getActiveCamera().getMVP();

		vkcv::PushConstants pushConstants = vkcv::pushConstants<glm::mat4>();
		pushConstants.appendDrawcall(mvp);

		const std::vector<vkcv::ImageHandle> renderTargets = { swapchainInput, depthBuffer };
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		core.recordDrawcallsToCmdStream(
			cmdStream,
			firstMeshPipeline,
			pushConstants,
			{ drawcall },
			renderTargets,
			windowHandle
		);
		
		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
	});
	
	return 0;
}

#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>

// Assumes the meshes use index buffers


void addMeshToIndirectDraw(const vkcv::asset::Scene &scene,
                           std::vector<uint8_t> &compiledVertexBuffer,
                           std::vector<uint8_t> &compiledIndexBuffer,
                           std::vector<vk::DrawIndexedIndirectCommand> &indexedIndirectCommands)
{
    for (const auto &mesh : scene.meshes)
    {
        for(auto &vertexGroupIndex : mesh.vertexGroups)
        {
            auto &vertexGroup = scene.vertexGroups[vertexGroupIndex];

            indexedIndirectCommands.emplace_back(static_cast<uint32_t>(vertexGroup.indexBuffer.data.size()),
                                                 1,
                                                 static_cast<uint32_t>(compiledIndexBuffer.size()),
                                                 0,
                                                 static_cast<uint32_t>(indexedIndirectCommands.size()));

            compiledVertexBuffer.insert(compiledVertexBuffer.end(),
                                        vertexGroup.vertexBuffer.data.begin(),
                                        vertexGroup.vertexBuffer.data.end());

            compiledIndexBuffer.insert(compiledIndexBuffer.end(),
                                       vertexGroup.indexBuffer.data.begin(),
                                       vertexGroup.indexBuffer.data.end());
        }
    }
}

int main(int argc, const char** argv) {
	const char* applicationName = "Indirect draw";

	uint32_t windowWidth = 800;
	uint32_t windowHeight = 600;

	vkcv::Window window = vkcv::Window::create(
		applicationName,
		windowWidth,
		windowHeight,
		false
	);
	
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
		window,
		applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		{ vk::QueueFlagBits::eGraphics ,vk::QueueFlagBits::eCompute , vk::QueueFlagBits::eTransfer },
		features
	);

	vkcv::asset::Scene asset_scene;
	const char* path = argc > 1 ? argv[1] : "resources/Sponza/Sponza.gltf";
	int result = vkcv::asset::loadScene(path, asset_scene);

	if (result == 1) {
		std::cout << "Loading Sponza successful!" << std::endl;
	} else {
		std::cerr << "Loading Sponza failed: " << result << std::endl;
		return 1;
	}
	assert(!asset_scene.vertexGroups.empty());
    if (asset_scene.textures.empty())
    {
        std::cerr << "Error. No textures found. Exiting." << std::endl;
        return EXIT_FAILURE;
    }

    const vkcv::AttachmentDescription present_color_attachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		core.getSwapchain().getFormat()
	);
	const vkcv::AttachmentDescription depth_attachment(
			vkcv::AttachmentOperation::STORE,
			vkcv::AttachmentOperation::CLEAR,
			vk::Format::eD32Sfloat
	);


	vkcv::PassConfig passDefinition({ present_color_attachment, depth_attachment });
	vkcv::PassHandle passHandle = core.createPass(passDefinition);
	if (!passHandle) {
		std::cerr << "Error. Could not create renderpass. Exiting." << std::endl;
		return EXIT_FAILURE;
	}


	vkcv::ShaderProgram sponzaProgram;
	vkcv::shader::GLSLCompiler compiler;
	compiler.compile(vkcv::ShaderStage::VERTEX, std::filesystem::path("resources/shaders/shader.vert"),
					 [&sponzaProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
                         sponzaProgram.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("resources/shaders/shader.frag"),
					 [&sponzaProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
                         sponzaProgram.addShader(shaderStage, path);
	});

    // vertex layout for the pipeline. (assumed to be) used by all sponza meshes.
    const std::vector<vkcv::VertexAttachment> vertexAttachments = sponzaProgram.getVertexAttachments();
    std::vector<vkcv::VertexBinding> bindings;
    for (size_t i = 0; i < vertexAttachments.size(); i++)
    {
        bindings.push_back(vkcv::VertexBinding(i, { vertexAttachments[i] }));
    }
    const vkcv::VertexLayout sponzaVertexLayout (bindings);

    // recreation of VertexBufferBindings YET AGAIN,
    // since these are used in the command buffer to bind and draw from the vertex shaders


    std::vector<vk::DrawIndexedIndirectCommand> indexedIndirectCommands;
    std::vector<uint8_t> compiledVertexBuffer;
    std::vector<uint8_t> compiledIndexBuffer;

    addMeshToIndirectDraw(asset_scene,
                          compiledVertexBuffer,
                          compiledIndexBuffer,
                          indexedIndirectCommands);

    vkcv::Buffer<vk::DrawIndexedIndirectCommand> indirectBuffer = core.createBuffer<vk::DrawIndexedIndirectCommand>(
            vkcv::BufferType::INDIRECT,
            indexedIndirectCommands.size() * sizeof(vk::DrawIndexedIndirectCommand),
            vkcv::BufferMemoryType::DEVICE_LOCAL);
    indirectBuffer.fill(indexedIndirectCommands);

    auto vkCompiledVertexBuffer = core.createBuffer<uint8_t>(
            vkcv::BufferType::VERTEX,
            compiledVertexBuffer.size(),
            vkcv::BufferMemoryType::DEVICE_LOCAL);
    vkCompiledVertexBuffer.fill(compiledVertexBuffer.data());

    auto vkCompiledIndexBuffer = core.createBuffer<uint8_t>(
            vkcv::BufferType::INDEX,
            compiledIndexBuffer.size(),
            vkcv::BufferMemoryType::DEVICE_LOCAL);
    vkCompiledIndexBuffer.fill(compiledIndexBuffer.data());

    std::vector<vkcv::asset::VertexAttribute> attributes = asset_scene.vertexGroups[0].vertexBuffer.attributes;
    std::sort(attributes.begin(),
              attributes.end(),
              [](const vkcv::asset::VertexAttribute& x, const vkcv::asset::VertexAttribute& y)
              {return static_cast<uint32_t>(x.type) < static_cast<uint32_t>(y.type);});

     const std::vector<vkcv::VertexBufferBinding> vertexBufferBindings = {
            vkcv::VertexBufferBinding(static_cast<vk::DeviceSize>(attributes[0].offset), vkCompiledVertexBuffer.getVulkanHandle()),
            vkcv::VertexBufferBinding(static_cast<vk::DeviceSize>(attributes[1].offset), vkCompiledVertexBuffer.getVulkanHandle()),
            vkcv::VertexBufferBinding(static_cast<vk::DeviceSize>(attributes[2].offset), vkCompiledVertexBuffer.getVulkanHandle()) };

    const vkcv::Mesh mesh(vertexBufferBindings, vkCompiledIndexBuffer.getVulkanHandle(), compiledIndexBuffer.size());


	//vkcv::DescriptorBindings descriptorBindings = sponzaProgram.getReflectedDescriptors().at(0);
	vkcv::DescriptorSetLayoutHandle descriptorSetLayout = core.createDescriptorSetLayout({});
	//vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(descriptorSetLayout);

    /*
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
    */

        /*
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
    */

	const vkcv::PipelineConfig sponzaPipelineConfig {
        sponzaProgram,
        UINT32_MAX,
        UINT32_MAX,
        passHandle,
        {sponzaVertexLayout},
		{ core.getDescriptorSetLayout(descriptorSetLayout).vulkanHandle },
		true
	};
	vkcv::PipelineHandle sponzaPipelineHandle = core.createGraphicsPipeline(sponzaPipelineConfig);
	
	if (!sponzaPipelineHandle) {
		std::cerr << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	





    vkcv::camera::CameraManager cameraManager(window);
    uint32_t camIndex0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	uint32_t camIndex1 = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);
	
	cameraManager.getCamera(camIndex0).setPosition(glm::vec3(0, 0, -3));

    vkcv::ImageHandle depthBuffer = core.createImage(vk::Format::eD32Sfloat, windowWidth, windowHeight, 1, false).getHandle();
    const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

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
		auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		
		start = end;
		cameraManager.update(0.000001 * static_cast<double>(deltatime.count()));
        vkcv::camera::Camera cam = cameraManager.getActiveCamera();

		vkcv::PushConstants pushConstants(0);

        /*
        for (const auto &mesh : asset_scene.meshes)
        {
            glm::mat4 modelMesh = glm::make_mat4(mesh.modelMatrix.data());
            pushConstants.appendDrawcall(cam.getProjection() * cam.getView() * modelMesh);
        }
        */

		const std::vector<vkcv::ImageHandle> renderTargets = { swapchainInput, depthBuffer };
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		core.recordIndexedIndirectDrawcallsToCmdStream(
			cmdStream,
			passHandle,
            sponzaPipelineHandle,
            pushConstants,
			mesh,
			renderTargets,
			indirectBuffer,
            1);

		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
		core.endFrame();
	}
	
	return 0;
}

#include <iostream>
#include <vkcv/Core.hpp>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
};

struct CompiledMaterial
{
    std::vector<vkcv::Image> baseColor;
    std::vector<vkcv::Image> metalRough;
    std::vector<vkcv::Image> normal;
    std::vector<vkcv::Image> occlusion;
};

std::vector<std::vector<Vertex>> interleaveScene(vkcv::asset::Scene scene)
{
	std::vector<std::vector<Vertex>> returnScene;

	for(auto& vertexGroup : scene.vertexGroups )
	{
		const vkcv::asset::VertexAttribute positionAttribute = vertexGroup.vertexBuffer.attributes[0];
		const vkcv::asset::VertexAttribute normalAttribute   = vertexGroup.vertexBuffer.attributes[1];
		const vkcv::asset::VertexAttribute uvAttribute       = vertexGroup.vertexBuffer.attributes[2];

		assert(positionAttribute.type   == vkcv::asset::PrimitiveType::POSITION);
		assert(normalAttribute.type     == vkcv::asset::PrimitiveType::NORMAL);
		assert(uvAttribute.type         == vkcv::asset::PrimitiveType::TEXCOORD_0);

		const uint64_t &verticesCount          = vertexGroup.numVertices;
		const std::vector<uint8_t> &vertexData = vertexGroup.vertexBuffer.data;

		std::vector<Vertex> vertices;
		vertices.reserve(verticesCount);

		const size_t positionStride = positionAttribute.stride == 0 ? sizeof(glm::vec3) : positionAttribute.stride;
		const size_t normalStride   = normalAttribute.stride   == 0 ? sizeof(glm::vec3) : normalAttribute.stride;
		const size_t uvStride       = uvAttribute.stride       == 0 ? sizeof(glm::vec2) : uvAttribute.stride;

		for(auto i = 0; i < verticesCount; i++)
		{
			const size_t positionOffset = positionAttribute.offset + positionStride * i;
			const size_t normalOffset   = normalAttribute.offset   + normalStride * i;
			const size_t uvOffset       = uvAttribute.offset       + uvStride * i;

			Vertex v;

			v.position = *reinterpret_cast<const glm::vec3*>(&(vertexData[positionOffset]));
			v.normal   = *reinterpret_cast<const glm::vec3*>(&(vertexData[normalOffset]));
			v.uv       = *reinterpret_cast<const glm::vec3*>(&(vertexData[uvOffset]));

			vertices.push_back(v);
		}
		returnScene.push_back(vertices);
	}
	assert(returnScene.size() == scene.vertexGroups.size());
	return returnScene;
}

// Assumes the meshes use index buffers
void compileMeshForIndirectDraw(vkcv::Core &core,
                                const vkcv::asset::Scene &scene,
                                std::vector<uint8_t> &compiledVertexBuffer,
                                std::vector<uint8_t> &compiledIndexBuffer,
                                CompiledMaterial &compiledMat,
                                std::vector<vk::DrawIndexedIndirectCommand> &indexedIndirectCommands)
{
    vkcv::Image pseudoImg = core.createImage(vk::Format::eR8G8B8A8Srgb, 2, 2);
    std::vector<uint8_t> pseudoData = {0, 0, 0, 0};
    pseudoImg.fill(pseudoData.data());
    pseudoImg.switchLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

	uint32_t vertexOffset = 0;
    for (const auto &mesh : scene.meshes)
    {
        for(auto &vertexGroupIndex : mesh.vertexGroups)
        {
            auto &vertexGroup = scene.vertexGroups[vertexGroupIndex];

            auto &material      = scene.materials[vertexGroup.materialIndex];

            if(material.baseColor == -1)
            {
                std::cout << "baseColor is -1! Pushing pseudo-texture!" << std::endl;
                compiledMat.baseColor.push_back(pseudoImg);
            }
            else
            {
                auto &baseColor     = scene.textures[material.baseColor];

                vkcv::Image baseColorImg = core.createImage(vk::Format::eR8G8B8A8Srgb, baseColor.w, baseColor.h);
                baseColorImg.fill(baseColor.data.data());
                baseColorImg.generateMipChainImmediate();
                baseColorImg.switchLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

                compiledMat.baseColor.push_back(baseColorImg);
            }


            //auto &metalRough    = scene.textures[material.metalRough];
            //auto &normal        = scene.textures[material.normal];
            //auto &occlusion     = scene.textures[material.occlusion];


            //vkcv::Image metalRoughImg = core.createImage(vk::Format::eR8G8B8A8Srgb, metalRough.w, metalRough.h);
            //vkcv::Image normalImg = core.createImage(vk::Format::eR8G8B8A8Srgb, normal.w, normal.h);
            //vkcv::Image occlusionImg = core.createImage(vk::Format::eR8G8B8A8Srgb, occlusion.w, occlusion.h);



            //metalRoughImg.fill(baseColor.data.data());
            //metalRoughImg.generateMipChainImmediate();
            //metalRoughImg.switchLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

            //normalImg.fill(baseColor.data.data());
            //normalImg.generateMipChainImmediate();
            //normalImg.switchLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

            //occlusionImg.fill(baseColor.data.data());
            //occlusionImg.generateMipChainImmediate();
            //occlusionImg.switchLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

            //compiledMat.metalRough.push_back(metalRoughImg);
            //compiledMat.normal.push_back(normalImg);
            //compiledMat.occlusion.push_back(occlusionImg);

            indexedIndirectCommands.emplace_back(static_cast<uint32_t>(vertexGroup.numIndices),
                                                 1,
                                                 static_cast<uint32_t>(compiledIndexBuffer.size() / 4),
                                                 vertexOffset,
                                                 static_cast<uint32_t>(indexedIndirectCommands.size()));

            vertexOffset += vertexGroup.numVertices;

            compiledVertexBuffer.insert(compiledVertexBuffer.end(),
                                        vertexGroup.vertexBuffer.data.begin(),
                                        vertexGroup.vertexBuffer.data.end());

			if(vertexGroup.indexBuffer.type == vkcv::asset::IndexType::UINT8)
            {
				for(auto index : vertexGroup.indexBuffer.data)
				{
                    uint32_t index32 = static_cast<uint32_t>(index);
                    uint8_t firstByte = index32;
                    uint8_t secondByte = index32 >> 8;
                    uint8_t thirdByte = index32 >> 16;
                    uint8_t fourthByte = index32 >> 24;

                    compiledIndexBuffer.push_back(firstByte);
                    compiledIndexBuffer.push_back(secondByte);
                    compiledIndexBuffer.push_back(thirdByte);
                    compiledIndexBuffer.push_back(fourthByte);
                }
			}
            else if(vertexGroup.indexBuffer.type == vkcv::asset::IndexType::UINT16)
            {
                for(auto i = 0; i < vertexGroup.indexBuffer.data.size(); i = i+2)
                {
                    uint16_t index16 = *reinterpret_cast<const uint16_t*>(&vertexGroup.indexBuffer.data[i]);
                    uint32_t index32 = static_cast<uint32_t>(index16);

                    uint8_t firstByte = index32;
                    uint8_t secondByte = index32 >> 8;
                    uint8_t thirdByte = index32 >> 16;
                    uint8_t fourthByte = index32 >> 24;

                    compiledIndexBuffer.push_back(firstByte);
                    compiledIndexBuffer.push_back(secondByte);
                    compiledIndexBuffer.push_back(thirdByte);
                    compiledIndexBuffer.push_back(fourthByte);
                }
			}
			else
			{
				compiledIndexBuffer.insert(compiledIndexBuffer.end(),
										   vertexGroup.indexBuffer.data.begin(),
										   vertexGroup.indexBuffer.data.end());
			}
        }
    }
}

int main(int argc, const char** argv) {
	const char* applicationName = "Indirect draw";

	uint32_t windowWidth = 800;
	uint32_t windowHeight = 600;

	vkcv::Features features;
	features.requireExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    features.requireFeature([](vk::PhysicalDeviceFeatures &features){
        features.setMultiDrawIndirect(true);
    });

    features.requireExtension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    features.requireExtensionFeature<vk::PhysicalDeviceDescriptorIndexingFeatures>(
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, [](vk::PhysicalDeviceDescriptorIndexingFeatures &features) {
                // features.setShaderInputAttachmentArrayDynamicIndexing(true);
                features.setShaderUniformTexelBufferArrayDynamicIndexing(true);
                features.setShaderStorageTexelBufferArrayDynamicIndexing(true);
                features.setShaderUniformBufferArrayNonUniformIndexing(true);
                features.setShaderSampledImageArrayNonUniformIndexing(true);
                features.setShaderStorageBufferArrayNonUniformIndexing(true);
                features.setShaderStorageImageArrayNonUniformIndexing(true);
                // features.setShaderInputAttachmentArrayNonUniformIndexing(true);
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

	vkcv::WindowHandle windowHandle = core.createWindow(applicationName,windowWidth,windowHeight,false);

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
		core.getSwapchain(windowHandle).getFormat()
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
	const vkcv::VertexLayout sponzaVertexLayout({ vkcv::VertexBinding(0, { vertexAttachments }) });

    std::vector<uint8_t> compiledVertexBuffer;
    std::vector<uint8_t> compiledIndexBuffer;
    CompiledMaterial compiledMaterial;
    std::vector<vk::DrawIndexedIndirectCommand> indexedIndirectCommands;

    compileMeshForIndirectDraw(core,
                               asset_scene,
                               compiledVertexBuffer,
                               compiledIndexBuffer,
                               compiledMaterial,
                               indexedIndirectCommands);

	std::vector<std::vector<Vertex>> interleavedVertices = interleaveScene(asset_scene);
	std::vector<Vertex> compiledInterleavedBuffer;
	for(auto& vertexGroup : interleavedVertices )
	{
		compiledInterleavedBuffer.insert(compiledInterleavedBuffer.end(),vertexGroup.begin(),vertexGroup.end());
	}

	auto compiledInterleavedVertexBuffer = core.createBuffer<Vertex>(
			vkcv::BufferType::VERTEX,
			compiledInterleavedBuffer.size(),
			vkcv::BufferMemoryType::DEVICE_LOCAL
			);
	compiledInterleavedVertexBuffer.fill(compiledInterleavedBuffer.data());

    auto vkCompiledIndexBuffer = core.createBuffer<uint8_t>(
            vkcv::BufferType::INDEX,
            compiledIndexBuffer.size(),
            vkcv::BufferMemoryType::DEVICE_LOCAL);
    vkCompiledIndexBuffer.fill(compiledIndexBuffer.data());

    vkcv::Buffer<vk::DrawIndexedIndirectCommand> indirectBuffer = core.createBuffer<vk::DrawIndexedIndirectCommand>(
            vkcv::BufferType::INDIRECT,
            indexedIndirectCommands.size() * sizeof(vk::DrawIndexedIndirectCommand),
            vkcv::BufferMemoryType::DEVICE_LOCAL);
    indirectBuffer.fill(indexedIndirectCommands);

	const std::vector<vkcv::VertexBufferBinding> vertexBufferBindings = {
			vkcv::VertexBufferBinding(static_cast<vk::DeviceSize> (0), compiledInterleavedVertexBuffer.getVulkanHandle() )
	};

	const vkcv::Mesh compiledMesh(vertexBufferBindings, vkCompiledIndexBuffer.getVulkanHandle(), 0, vkcv::IndexBitCount::Bit32);

    //assert(compiledMaterial.baseColor.size() == compiledMaterial.metalRough.size());

	vkcv::DescriptorBindings descriptorBindings = sponzaProgram.getReflectedDescriptors().at(0);
    descriptorBindings[1].descriptorCount = compiledMaterial.baseColor.size();
    //descriptorBindings[2].descriptorCount = compiledMaterial.metalRough.size();
    //descriptorBindings[3].descriptorCount = compiledMaterial.normal.size();
    //descriptorBindings[4].descriptorCount = compiledMaterial.occlusion.size();

	vkcv::DescriptorSetLayoutHandle descriptorSetLayout = core.createDescriptorSetLayout(descriptorBindings);
	vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(descriptorSetLayout);

    vkcv::SamplerHandle standardSampler = core.createSampler(
            vkcv::SamplerFilterType::LINEAR,
            vkcv::SamplerFilterType::LINEAR,
            vkcv::SamplerMipmapMode::LINEAR,
            vkcv::SamplerAddressMode::REPEAT
    );

    std::vector<vkcv::SampledImageDescriptorWrite> textureArrayWrites;
    for(uint32_t i = 0; i < compiledMaterial.baseColor.size(); i++)
    {
        vkcv::SampledImageDescriptorWrite baseColorWrite(1, compiledMaterial.baseColor[i].getHandle(), 0, false, i);
        //vkcv::SampledImageDescriptorWrite metalRoughWrite(1, compiledMaterial.metalRough[i].getHandle(), 0, false, i);
        //vkcv::SampledImageDescriptorWrite normalWrite(2, compiledMaterial.normal[i].getHandle(), 0, false, i);
        //vkcv::SampledImageDescriptorWrite occlusionWrite(3, compiledMaterial.occlusion[i].getHandle(), 0, false, i);

        textureArrayWrites.push_back(baseColorWrite);
        //textureArrayWrites.push_back(metalRoughWrite);
        //textureArrayWrites.push_back(normalWrite);
        //textureArrayWrites.push_back(occlusionWrite);
    }

    vkcv::DescriptorWrites setWrites;
    setWrites.sampledImageWrites	= textureArrayWrites;
    setWrites.samplerWrites			= { vkcv::SamplerDescriptorWrite(0, standardSampler) };
    core.writeDescriptorSet(descriptorSet, setWrites);


	const vkcv::GraphicsPipelineConfig sponzaPipelineConfig {
        sponzaProgram,
        UINT32_MAX,
        UINT32_MAX,
        passHandle,
        {sponzaVertexLayout},
		{ core.getDescriptorSetLayout(descriptorSetLayout).vulkanHandle },
		true
	};
	vkcv::GraphicsPipelineHandle sponzaPipelineHandle = core.createGraphicsPipeline(sponzaPipelineConfig);
	
	if (!sponzaPipelineHandle) {
		std::cerr << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	



    vkcv::camera::CameraManager cameraManager(core.getWindow(windowHandle));
    uint32_t camIndex0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	uint32_t camIndex1 = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);
	
	cameraManager.getCamera(camIndex0).setPosition(glm::vec3(0, 0, -3));
	cameraManager.getCamera(camIndex0).setNearFar(0.1f, 20.f);

    vkcv::ImageHandle depthBuffer = core.createImage(vk::Format::eD32Sfloat, windowWidth, windowHeight, 1, false).getHandle();
    const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

    auto start = std::chrono::system_clock::now();

	while (vkcv::Window::hasOpenWindow()) {
        vkcv::Window::pollEvents();
		
		if(core.getWindow(windowHandle).getHeight() == 0 || core.getWindow(windowHandle).getWidth() == 0)
			continue;
		
		uint32_t swapchainWidth, swapchainHeight;
		if (!core.beginFrame(swapchainWidth, swapchainHeight, windowHandle)) {
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

		vkcv::PushConstants pushConstants(sizeof(glm::mat4));

		for( auto& mesh : asset_scene.meshes)
		{
			glm::mat4 modelMatrix = glm::make_mat4(mesh.modelMatrix.data());
			pushConstants.appendDrawcall(cam.getProjection() * cam.getView() * modelMatrix);
		}

		const std::vector<vkcv::ImageHandle> renderTargets = { swapchainInput, depthBuffer };
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		core.recordIndexedIndirectDrawcallsToCmdStream(
			cmdStream,
			passHandle,
            sponzaPipelineHandle,
            pushConstants,
            descriptorSet,
            compiledMesh,
			renderTargets,
			indirectBuffer,
            indexedIndirectCommands.size(),
			windowHandle);

		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
		core.endFrame(windowHandle);
	}
	
	return 0;
}

#include <iostream>
#include <vkcv/Buffer.hpp>
#include <vkcv/Core.hpp>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include <vkcv/gui/GUI.hpp>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>

struct Plane {
    glm::vec3   pointOnPlane;
    float       padding0;
    glm::vec3   normal;
    float       padding1;
};

struct CameraPlanes {
    Plane planes[6];
};

CameraPlanes computeCameraPlanes(const vkcv::camera::Camera& camera) {
    const float     fov     = camera.getFov();
    const glm::vec3 pos     = camera.getPosition();
    const float     ratio   = camera.getRatio();
    const glm::vec3 forward = glm::normalize(camera.getFront());
    float near;
    float far;
    camera.getNearFar(near, far);

    glm::vec3 up    = glm::vec3(0, -1, 0);
    glm::vec3 right = glm::normalize(glm::cross(forward, up));
    up              = glm::cross(forward, right);

    const glm::vec3 nearCenter      = pos + forward * near;
    const glm::vec3 farCenter       = pos + forward * far;

    const float tanFovHalf          = glm::tan(fov / 2);

    const glm::vec3 nearUpCenter    = nearCenter + up    * tanFovHalf * near;
    const glm::vec3 nearDownCenter  = nearCenter - up    * tanFovHalf * near;
    const glm::vec3 nearRightCenter = nearCenter + right * tanFovHalf * near * ratio;
    const glm::vec3 nearLeftCenter  = nearCenter - right * tanFovHalf * near * ratio;

    const glm::vec3 farUpCenter     = farCenter + up    * tanFovHalf * far;
    const glm::vec3 farDownCenter   = farCenter - up    * tanFovHalf * far;
    const glm::vec3 farRightCenter  = farCenter + right * tanFovHalf * far * ratio;
    const glm::vec3 farLeftCenter   = farCenter - right * tanFovHalf * far * ratio;

    CameraPlanes cameraPlanes;
    // near
    cameraPlanes.planes[0].pointOnPlane = nearCenter;
    cameraPlanes.planes[0].normal       = -forward;
    // far
    cameraPlanes.planes[1].pointOnPlane = farCenter;
    cameraPlanes.planes[1].normal       = forward;

    // top
    cameraPlanes.planes[2].pointOnPlane = nearUpCenter;
    cameraPlanes.planes[2].normal       = glm::normalize(glm::cross(farUpCenter - nearUpCenter, right));
    // bot
    cameraPlanes.planes[3].pointOnPlane = nearDownCenter;
    cameraPlanes.planes[3].normal       = glm::normalize(glm::cross(right, farDownCenter - nearDownCenter));

    // right
    cameraPlanes.planes[4].pointOnPlane = nearRightCenter;
    cameraPlanes.planes[4].normal       = glm::normalize(glm::cross(up, farRightCenter - nearRightCenter));
    // left
    cameraPlanes.planes[5].pointOnPlane = nearLeftCenter;
    cameraPlanes.planes[5].normal       = glm::normalize(glm::cross(farLeftCenter - nearLeftCenter, up));

    return cameraPlanes;
}

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

void interleaveScene(vkcv::asset::Scene scene,
                     std::vector<std::vector<Vertex>> &interleavedVertexBuffers,
                     std::vector<glm::vec4> &boundingBoxBuffers)
{
    for(const auto &mesh : scene.meshes)
    {
        for(auto vertexGroupIndex : mesh.vertexGroups)
        {
			// Sort attributes to fix it!
			auto& attributes = scene.vertexGroups[vertexGroupIndex].vertexBuffer.attributes;
	
			std::sort(attributes.begin(), attributes.end(), [](const vkcv::asset::VertexAttribute& x, const vkcv::asset::VertexAttribute& y) {
				return static_cast<uint32_t>(x.type) < static_cast<uint32_t>(y.type);
			});
			
            const auto &vertexGroup = scene.vertexGroups[vertexGroupIndex];

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

            glm::vec3 max_pos(-std::numeric_limits<float>::max());
            glm::vec3 min_pos(std::numeric_limits<float>::max());

            for(size_t i = 0; i < verticesCount; i++)
            {
                const size_t positionOffset = positionAttribute.offset + positionStride * i;
                const size_t normalOffset   = normalAttribute.offset   + normalStride * i;
                const size_t uvOffset       = uvAttribute.offset       + uvStride * i;

                Vertex v;

                v.position = *reinterpret_cast<const glm::vec3*>(&(vertexData[positionOffset]));
                v.normal   = *reinterpret_cast<const glm::vec3*>(&(vertexData[normalOffset]));
                v.uv       = *reinterpret_cast<const glm::vec3*>(&(vertexData[uvOffset]));

                glm::vec3 posWorld = glm::make_mat4(mesh.modelMatrix.data()) * glm::vec4(v.position, 1);

                max_pos.x = glm::max(max_pos.x, posWorld.x);
                max_pos.y = glm::max(max_pos.y, posWorld.y);
                max_pos.z = glm::max(max_pos.z, posWorld.z);

                min_pos.x = glm::min(min_pos.x, posWorld.x);
                min_pos.y = glm::min(min_pos.y, posWorld.y);
                min_pos.z = glm::min(min_pos.z, posWorld.z);

                vertices.push_back(v);
            }

            const glm::vec3 boundingPosition = (max_pos + min_pos) / 2.0f;
            const float radius = glm::distance(max_pos, min_pos) / 2.0f;

            boundingBoxBuffers.emplace_back(boundingPosition.x,
                                            boundingPosition.y,
                                            boundingPosition.z,
                                            radius);

            interleavedVertexBuffers.push_back(vertices);
        }
    }
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
	
	auto mipStream = core.createCommandStream(vkcv::QueueType::Graphics);
	pseudoImg.recordMipChainGeneration(mipStream, core.getDownsampler());

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
				baseColorImg.recordMipChainGeneration(mipStream, core.getDownsampler());

                compiledMat.baseColor.push_back(baseColorImg);
            }

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
                for (size_t i = 0; i < vertexGroup.indexBuffer.data.size(); i += 2)
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
	
	core.submitCommandStream(mipStream, false);
}

int main(int argc, const char** argv) {
	const char* applicationName = "Indirect draw";

	vkcv::Features features;
	features.requireExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    features.requireFeature([](vk::PhysicalDeviceFeatures &features){
        features.setMultiDrawIndirect(true);
    });

	features.requireExtension(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
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

	vkcv::WindowHandle windowHandle = core.createWindow(applicationName,800,600,true);
	vkcv::Window& window = core.getWindow(windowHandle);

    vkcv::gui::GUI gui (core, windowHandle);

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

    const vkcv::AttachmentDescription present_color_attachment (
			core.getSwapchainFormat(window.getSwapchain()),
			vkcv::AttachmentOperation::CLEAR,
			vkcv::AttachmentOperation::STORE
	);
	
	const vkcv::AttachmentDescription depth_attachment (
			vk::Format::eD32Sfloat,
			vkcv::AttachmentOperation::CLEAR,
			vkcv::AttachmentOperation::STORE
	);

	vkcv::PassConfig passDefinition({ present_color_attachment, depth_attachment }, vkcv::Multisampling::None);
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

    vkcv::ShaderProgram cullingProgram;
    compiler.compile(vkcv::ShaderStage::COMPUTE, std::filesystem::path("resources/shaders/culling.comp"),
                     [&cullingProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
        cullingProgram.addShader(shaderStage, path);
    });

    // vertex layout for the pipeline. (assumed to be) used by all sponza meshes.
    const std::vector<vkcv::VertexAttachment> vertexAttachments = sponzaProgram.getVertexAttachments();
	const vkcv::VertexLayout sponzaVertexLayout {
		{ vkcv::createVertexBinding(0, { vertexAttachments }) }
	};

    std::vector<uint8_t> compiledVertexBuffer; // IGNORED, since the vertex buffer is not interleaved!

    std::vector<uint8_t> compiledIndexBuffer;
    CompiledMaterial compiledMaterial;
    std::vector<vk::DrawIndexedIndirectCommand> indexedIndirectCommands;

    compileMeshForIndirectDraw(core,
                               asset_scene,
                               compiledVertexBuffer,
                               compiledIndexBuffer,
                               compiledMaterial,
                               indexedIndirectCommands);

	std::vector<std::vector<Vertex>> interleavedVertices;
    std::vector<glm::vec4> compiledBoundingBoxBuffer;
    interleaveScene(asset_scene,
                    interleavedVertices,
                    compiledBoundingBoxBuffer);

	std::vector<Vertex> compiledInterleavedVertexBuffer;
	for(auto& vertexGroup : interleavedVertices )
	{
        compiledInterleavedVertexBuffer.insert(compiledInterleavedVertexBuffer.end(),vertexGroup.begin(),vertexGroup.end());
	}

	auto vkCompiledVertexBuffer = vkcv::buffer<Vertex>(
			core,
			vkcv::BufferType::VERTEX,
            compiledInterleavedVertexBuffer.size(),
			vkcv::BufferMemoryType::DEVICE_LOCAL);
    vkCompiledVertexBuffer.fill(compiledInterleavedVertexBuffer.data());

    auto vkCompiledIndexBuffer = vkcv::buffer<uint8_t>(
			core,
            vkcv::BufferType::INDEX,
            compiledIndexBuffer.size(),
            vkcv::BufferMemoryType::DEVICE_LOCAL);
    vkCompiledIndexBuffer.fill(compiledIndexBuffer.data());

    vkcv::Buffer<vk::DrawIndexedIndirectCommand> indirectBuffer = vkcv::buffer<vk::DrawIndexedIndirectCommand>(
			core,
            vkcv::BufferType::INDIRECT,
            indexedIndirectCommands.size(),
            vkcv::BufferMemoryType::DEVICE_LOCAL);
    indirectBuffer.fill(indexedIndirectCommands);

    auto boundingBoxBuffer = vkcv::buffer<glm::vec4>(
			core,
            vkcv::BufferType::STORAGE,
            compiledBoundingBoxBuffer.size());
    boundingBoxBuffer.fill(compiledBoundingBoxBuffer);

    std::vector<glm::mat4> modelMatrix;
	for( auto& mesh : asset_scene.meshes)
	{
		modelMatrix.push_back(glm::make_mat4(mesh.modelMatrix.data()));
	}
	vkcv::Buffer<glm::mat4> modelBuffer = vkcv::buffer<glm::mat4>(
			core,
			vkcv::BufferType::STORAGE,
			modelMatrix.size(),
			vkcv::BufferMemoryType::DEVICE_LOCAL
			);
	modelBuffer.fill(modelMatrix);

	const std::vector<vkcv::VertexBufferBinding> vertexBufferBindings = {
			vkcv::VertexBufferBinding(static_cast<vk::DeviceSize> (0), vkCompiledVertexBuffer.getVulkanHandle() )
	};

	const vkcv::Mesh compiledMesh(vertexBufferBindings, vkCompiledIndexBuffer.getVulkanHandle(), 0, vkcv::IndexBitCount::Bit32);

    //assert(compiledMaterial.baseColor.size() == compiledMaterial.metalRough.size());

	vkcv::DescriptorBindings descriptorBindings = sponzaProgram.getReflectedDescriptors().at(0);
    descriptorBindings[2].descriptorCount = compiledMaterial.baseColor.size();

	vkcv::DescriptorSetLayoutHandle descriptorSetLayout = core.createDescriptorSetLayout(descriptorBindings);
	vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(descriptorSetLayout);

    vkcv::SamplerHandle standardSampler = core.createSampler(
            vkcv::SamplerFilterType::LINEAR,
            vkcv::SamplerFilterType::LINEAR,
            vkcv::SamplerMipmapMode::LINEAR,
            vkcv::SamplerAddressMode::REPEAT
    );
	
	vkcv::DescriptorWrites setWrites;
	
    std::vector<vkcv::SampledImageDescriptorWrite> textureArrayWrites;
    for(uint32_t i = 0; i < compiledMaterial.baseColor.size(); i++)
    {
		setWrites.writeSampledImage(2, compiledMaterial.baseColor[i].getHandle(), 0, false, i);
    }
    
    setWrites.writeSampler(0, standardSampler);
	setWrites.writeStorageBuffer(1, modelBuffer.getHandle());
    core.writeDescriptorSet(descriptorSet, setWrites);

	vkcv::GraphicsPipelineHandle sponzaPipelineHandle = core.createGraphicsPipeline(
			vkcv::GraphicsPipelineConfig(
					sponzaProgram,
					passHandle,
					{ sponzaVertexLayout },
					{ descriptorSetLayout }
			)
	);
	
	if (!sponzaPipelineHandle) {
		std::cerr << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

    vkcv::DescriptorBindings cullingBindings = cullingProgram.getReflectedDescriptors().at(0);
    vkcv::DescriptorSetLayoutHandle cullingSetLayout = core.createDescriptorSetLayout(cullingBindings);
    vkcv::DescriptorSetHandle cullingDescSet = core.createDescriptorSet(cullingSetLayout);

    vkcv::Buffer<CameraPlanes> cameraPlaneBuffer = vkcv::buffer<CameraPlanes>(
			core,
            vkcv::BufferType::UNIFORM,
            1);

    vkcv::DescriptorWrites cullingWrites;
	cullingWrites.writeStorageBuffer(1, indirectBuffer.getHandle());
	cullingWrites.writeStorageBuffer(2, boundingBoxBuffer.getHandle());
    cullingWrites.writeUniformBuffer(0, cameraPlaneBuffer.getHandle());
    core.writeDescriptorSet(cullingDescSet, cullingWrites);


    const vkcv::ComputePipelineConfig computeCullingConfig {
        cullingProgram,
        {cullingSetLayout}
    };
    vkcv::ComputePipelineHandle cullingPipelineHandle = core.createComputePipeline(computeCullingConfig);
    if (!cullingPipelineHandle) {
        std::cerr << "Error. Could not create culling pipeline. Exiting." << std::endl;
        return EXIT_FAILURE;
    }

    vkcv::camera::CameraManager cameraManager (window);
    uint32_t camIndex0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	
	cameraManager.getCamera(camIndex0).setPosition(glm::vec3(0, 0, -3));
	cameraManager.getCamera(camIndex0).setNearFar(0.1f, 20.f);

    vkcv::ImageHandle depthBuffer;
	
    const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

    auto start = std::chrono::system_clock::now();

    float ceiledDispatchCount = static_cast<float>(indexedIndirectCommands.size()) / 64.0f;
    ceiledDispatchCount = std::ceil(ceiledDispatchCount);
    const vkcv::DispatchSize dispatchCount = static_cast<uint32_t>(ceiledDispatchCount);

    vkcv::DescriptorSetUsage cullingUsage(0, cullingDescSet, {});
    vkcv::PushConstants emptyPushConstant(0);

    bool updateFrustumPlanes    = true;

    while (vkcv::Window::hasOpenWindow()) {
        vkcv::Window::pollEvents();
		
		if (window.getHeight() == 0 || window.getWidth() == 0)
			continue;
		
		uint32_t swapchainWidth, swapchainHeight;
		if (!core.beginFrame(swapchainWidth, swapchainHeight, windowHandle)) {
			continue;
		}
		
		if ((!depthBuffer) ||
			(swapchainWidth != core.getImageWidth(depthBuffer)) ||
			(swapchainHeight != core.getImageHeight(depthBuffer))) {
			depthBuffer = core.createImage(vk::Format::eD32Sfloat, swapchainWidth, swapchainHeight).getHandle();
		}
  
		auto end = std::chrono::system_clock::now();
		auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		
		start = end;
		cameraManager.update(0.000001 * static_cast<double>(deltatime.count()));
        vkcv::camera::Camera cam = cameraManager.getActiveCamera();
		vkcv::PushConstants pushConstants = vkcv::pushConstants<glm::mat4>();
		pushConstants.appendDrawcall(cam.getProjection() * cam.getView());

        if (updateFrustumPlanes) {
            const CameraPlanes cameraPlanes = computeCameraPlanes(cam);
            cameraPlaneBuffer.fill({ cameraPlanes });
        }

		const std::vector<vkcv::ImageHandle> renderTargets = { swapchainInput, depthBuffer };
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

        core.recordComputeDispatchToCmdStream(
				cmdStream,
				cullingPipelineHandle,
				dispatchCount,
				{cullingUsage},
				emptyPushConstant
		);

        core.recordBufferMemoryBarrier(cmdStream, indirectBuffer.getHandle());

		core.recordIndexedIndirectDrawcallsToCmdStream(
			cmdStream,
            sponzaPipelineHandle,
            pushConstants,
            descriptorSet,
            compiledMesh,
			renderTargets,
			indirectBuffer.getHandle(),
            indexedIndirectCommands.size(),
			windowHandle
		);

		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);

		auto stop = std::chrono::system_clock::now();
		auto kektime = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        gui.beginGUI();

        ImGui::Begin("Settings");
        ImGui::Checkbox("Update frustum culling", &updateFrustumPlanes);
		ImGui::Text("Deltatime %fms, %f", 0.001 * static_cast<double>(kektime.count()), 1/(0.000001 * static_cast<double>(kektime.count())));

        ImGui::End();

        gui.endGUI();

		core.endFrame(windowHandle);
	}
	
	return 0;
}

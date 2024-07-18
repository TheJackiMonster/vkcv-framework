#include <iostream>
#include <vkcv/Buffer.hpp>
#include <vkcv/Core.hpp>
#include <vkcv/Pass.hpp>
#include <vkcv/Sampler.hpp>
#include <vkcv/Image.hpp>
#include <vkcv/camera/CameraManager.hpp>
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
                     std::vector<glm::vec4> &boundingBoxBuffers) {
	
    for(const auto &mesh : scene.meshes) {
        for(auto vertexGroupIndex : mesh.vertexGroups) {
            const auto &vertexGroup = scene.vertexGroups[vertexGroupIndex];

            const vkcv::asset::VertexAttribute* positionAttribute = nullptr;
            const vkcv::asset::VertexAttribute* normalAttribute   = nullptr;
            const vkcv::asset::VertexAttribute* uvAttribute       = nullptr;
			
			for (const auto& attribute : vertexGroup.vertexBuffer.attributes) {
				switch (attribute.type) {
					case vkcv::asset::PrimitiveType::POSITION:
						positionAttribute = &attribute;
						break;
					case vkcv::asset::PrimitiveType::NORMAL:
						normalAttribute = &attribute;
						break;
					case vkcv::asset::PrimitiveType::TEXCOORD_0:
						uvAttribute = &attribute;
						break;
					default:
						break;
				}
			}
	
			assert(positionAttribute && normalAttribute && uvAttribute);

            const uint64_t &verticesCount          = vertexGroup.numVertices;
            const std::vector<uint8_t> &vertexData = vertexGroup.vertexBuffer.data;

            std::vector<Vertex> vertices;
            vertices.reserve(verticesCount);

            const size_t positionStride = positionAttribute->stride == 0 ? sizeof(glm::vec3) : positionAttribute->stride;
            const size_t normalStride   = normalAttribute->stride   == 0 ? sizeof(glm::vec3) : normalAttribute->stride;
            const size_t uvStride       = uvAttribute->stride       == 0 ? sizeof(glm::vec2) : uvAttribute->stride;

            glm::vec3 max_pos(-std::numeric_limits<float>::max());
            glm::vec3 min_pos(std::numeric_limits<float>::max());

            for(size_t i = 0; i < verticesCount; i++)
            {
                const size_t positionOffset = positionAttribute->offset + positionStride * i;
                const size_t normalOffset   = normalAttribute->offset   + normalStride * i;
                const size_t uvOffset       = uvAttribute->offset       + uvStride * i;

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
    vkcv::Image pseudoImg = vkcv::image(core, vk::Format::eR8G8B8A8Srgb, 2, 2);
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

                vkcv::Image baseColorImg = vkcv::image(core, vk::Format::eR8G8B8A8Srgb, baseColor.w, baseColor.h);
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
	const std::string applicationName = "Indirect draw";

    vkcv_log(vkcv::LogLevel::TIME, "Startup");

	vkcv::Features features;
	features.requireExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    features.requireFeature([](vk::PhysicalDeviceFeatures &features){
        features.setMultiDrawIndirect(true);
    });

	features.requireExtension(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
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

    features.tryExtensionFeature<vk::PhysicalDeviceHostImageCopyFeaturesEXT>(
        VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME, [](vk::PhysicalDeviceHostImageCopyFeaturesEXT& features) {
				features.setHostImageCopy(true);
			}
    );

    vkcv_log(vkcv::LogLevel::TIME, "Features configured");

	vkcv::Core core = vkcv::Core::create(
		applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		{ vk::QueueFlagBits::eGraphics ,vk::QueueFlagBits::eCompute , vk::QueueFlagBits::eTransfer },
		features
	);

    vkcv_log(vkcv::LogLevel::TIME, "Core created");

	vkcv::WindowHandle windowHandle = core.createWindow(applicationName,800,600,true);
	vkcv::Window& window = core.getWindow(windowHandle);

    vkcv_log(vkcv::LogLevel::TIME, "Window created");

    vkcv::gui::GUI gui (core, windowHandle);

    vkcv_log(vkcv::LogLevel::TIME, "GUI initialized");

    vkcv::asset::Scene asset_scene;
	const char* path = argc > 1 ? argv[1] : "resources/Sponza/Sponza.gltf";
	int result = vkcv::asset::loadScene(path, asset_scene);

    vkcv_log(vkcv::LogLevel::TIME, "Scene loaded");

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

    vkcv::PassHandle passHandle = vkcv::passSwapchain(
			core,
			window.getSwapchain(),
			{ vk::Format::eUndefined, vk::Format::eD32Sfloat }
	);
	
	if (!passHandle) {
		std::cerr << "Error. Could not create renderpass. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

    vkcv_log(vkcv::LogLevel::TIME, "Scene verified");

	vkcv::ShaderProgram sponzaProgram;
	vkcv::shader::GLSLCompiler compiler;
    compiler.compileProgram(sponzaProgram, {
        { vkcv::ShaderStage::VERTEX, std::filesystem::path("resources/shaders/shader.vert") },
        { vkcv::ShaderStage::FRAGMENT, std::filesystem::path("resources/shaders/shader.frag") },
    }, nullptr);

    vkcv::ShaderProgram cullingProgram;
    compiler.compileProgram(cullingProgram, {
        { vkcv::ShaderStage::COMPUTE, std::filesystem::path("resources/shaders/culling.comp") },
    }, nullptr);

    vkcv_log(vkcv::LogLevel::TIME, "Shaders compiled");

    // vertex layout for the pipeline. (assumed to be) used by all sponza meshes.
    const std::vector<vkcv::VertexAttachment> vertexAttachments = sponzaProgram.getVertexAttachments();
	const vkcv::VertexLayout sponzaVertexLayout {
		{ vkcv::createVertexBinding(0, { vertexAttachments }) }
	};

    vkcv_log(vkcv::LogLevel::TIME, "Vertex layout configured");

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
    
    vkcv_log(vkcv::LogLevel::TIME, "Mesh compiled");

	std::vector<std::vector<Vertex>> interleavedVertices;
    std::vector<glm::vec4> compiledBoundingBoxBuffer;
    interleaveScene(asset_scene,
                    interleavedVertices,
                    compiledBoundingBoxBuffer);
    
    vkcv_log(vkcv::LogLevel::TIME, "Scene interleaved");

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

    vkcv_log(vkcv::LogLevel::TIME, "Buffers filled");

	const std::vector<vkcv::VertexBufferBinding> vertexBufferBindings = {
			vkcv::vertexBufferBinding(vkCompiledVertexBuffer)
	};
	
	vkcv::VertexData vertexData (vertexBufferBindings);
	vertexData.setIndexBuffer(vkCompiledIndexBuffer.getHandle(), vkcv::IndexBitCount::Bit32);

    //assert(compiledMaterial.baseColor.size() == compiledMaterial.metalRough.size());

	vkcv::DescriptorBindings descriptorBindings = sponzaProgram.getReflectedDescriptors().at(0);
    descriptorBindings[2].descriptorCount = compiledMaterial.baseColor.size();

	vkcv::DescriptorSetLayoutHandle descriptorSetLayout = core.createDescriptorSetLayout(descriptorBindings);
	vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(descriptorSetLayout);

    vkcv::SamplerHandle standardSampler = vkcv::samplerLinear(core);
	
	vkcv::DescriptorWrites setWrites;
	
    std::vector<vkcv::SampledImageDescriptorWrite> textureArrayWrites;
    for(uint32_t i = 0; i < compiledMaterial.baseColor.size(); i++)
    {
		setWrites.writeSampledImage(2, compiledMaterial.baseColor[i].getHandle(), 0, false, i);
    }
    
    setWrites.writeSampler(0, standardSampler);
	setWrites.writeStorageBuffer(1, modelBuffer.getHandle());
    core.writeDescriptorSet(descriptorSet, setWrites);

    vkcv_log(vkcv::LogLevel::TIME, "DescriptorSets written");

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

    vkcv_log(vkcv::LogLevel::TIME, "Graphics pipeline created");

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

    vkcv_log(vkcv::LogLevel::TIME, "Culling descriptor set written");

    const vkcv::ComputePipelineConfig computeCullingConfig {
        cullingProgram,
        {cullingSetLayout}
    };
    vkcv::ComputePipelineHandle cullingPipelineHandle = core.createComputePipeline(computeCullingConfig);
    if (!cullingPipelineHandle) {
        std::cerr << "Error. Could not create culling pipeline. Exiting." << std::endl;
        return EXIT_FAILURE;
    }

    vkcv_log(vkcv::LogLevel::TIME, "Compute pipeline created");

    vkcv::camera::CameraManager cameraManager (window);
    auto camHandle = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	
	cameraManager.getCamera(camHandle).setPosition(glm::vec3(0, 0, -3));
	cameraManager.getCamera(camHandle).setNearFar(0.1f, 20.f);

    vkcv::ImageHandle depthBuffer;
	
    const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

    float ceiledDispatchCount = static_cast<float>(indexedIndirectCommands.size()) / 64.0f;
    ceiledDispatchCount = std::ceil(ceiledDispatchCount);
    const vkcv::DispatchSize dispatchCount = static_cast<uint32_t>(ceiledDispatchCount);

    vkcv::DescriptorSetUsage cullingUsage = vkcv::useDescriptorSet(0, cullingDescSet);
    vkcv::PushConstants emptyPushConstant(0);

    bool updateFrustumPlanes    = true;
	
	core.run([&](const vkcv::WindowHandle &windowHandle, double t, double dt,
				 uint32_t swapchainWidth, uint32_t swapchainHeight) {
		if ((!depthBuffer) ||
			(swapchainWidth != core.getImageWidth(depthBuffer)) ||
			(swapchainHeight != core.getImageHeight(depthBuffer))) {
			depthBuffer = core.createImage(
					vk::Format::eD32Sfloat,
					vkcv::ImageConfig(
							swapchainWidth,
							swapchainHeight
					)
			);
		}
  
		cameraManager.update(dt);
		
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

		vkcv::IndirectDrawcall drawcall (
				indirectBuffer.getHandle(),
				vertexData,
				indexedIndirectCommands.size()
		);
		
		drawcall.useDescriptorSet(0, descriptorSet);
		
		core.recordIndirectDrawcallsToCmdStream(
			cmdStream,
            sponzaPipelineHandle,
            pushConstants,
			{ drawcall },
			renderTargets,
			windowHandle
		);

		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
		
        gui.beginGUI();

        ImGui::Begin("Settings");
        ImGui::Checkbox("Update frustum culling", &updateFrustumPlanes);
		ImGui::Text("Deltatime %fms, %f", dt * 1000, 1/dt);

        ImGui::End();

        gui.endGUI();
	});
	
	return 0;
}

#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>

#include <vkcv/shader/GLSLCompiler.hpp>
#include <vkcv/gui/GUI.hpp>
#include <vkcv/asset/asset_loader.hpp>
#include "MeshStruct.hpp"
#include <Map>

struct Vertex {
	glm::vec3   position;
	float       padding0;
	glm::vec3   normal;
	float       padding1;
};

struct Meshlet {
    uint32_t vertexOffset;
    uint32_t vertexCount;
    uint32_t indexOffset;
    uint32_t indexCount;
};

std::vector<Vertex> convertToVertices(
	const std::vector<uint8_t>&         vertexData,
	const uint64_t                      vertexCount,
	const vkcv::asset::VertexAttribute& positionAttribute,
	const vkcv::asset::VertexAttribute& normalAttribute) {

	assert(positionAttribute.type   == vkcv::asset::PrimitiveType::POSITION);
	assert(normalAttribute.type     == vkcv::asset::PrimitiveType::NORMAL);

	std::vector<Vertex> vertices;
	vertices.reserve(vertexCount);

	const size_t positionStepSize   = positionAttribute.stride == 0 ? sizeof(glm::vec3) : positionAttribute.stride;
	const size_t normalStepSize     = normalAttribute.stride   == 0 ? sizeof(glm::vec3) : normalAttribute.stride;

	for (int i = 0; i < vertexCount; i++) {
		Vertex v;

		const size_t positionOffset = positionAttribute.offset  + positionStepSize  * i;
		const size_t normalOffset   = normalAttribute.offset    + normalStepSize    * i;

		v.position  = *reinterpret_cast<const glm::vec3*>(&(vertexData[positionOffset]));
		v.normal    = *reinterpret_cast<const glm::vec3*>(&(vertexData[normalOffset]));
		vertices.push_back(v);
	}

	return vertices;
}

struct MeshShaderModelData {
	std::vector<Vertex>     vertices;
	std::vector<uint32_t>   localIndices;
	std::vector<Meshlet>    meshlets;
};

MeshShaderModelData createMeshShaderModelData(
	const std::vector<Vertex>&      inVertices,
	const std::vector<uint32_t>&    inIndices) {

	MeshShaderModelData data;
	size_t currentIndex = 0;

	const size_t maxVerticesPerMeshlet = 64;
	const size_t maxIndicesPerMeshlet  = 126 * 3;

	bool indicesAreLeft = true;

	while (indicesAreLeft) {
		Meshlet meshlet;

		meshlet.indexCount  = 0;
		meshlet.vertexCount = 0;

		meshlet.indexOffset  = data.localIndices.size();
		meshlet.vertexOffset = data.vertices.size();

        std::map<uint32_t, uint32_t> globalToLocalIndexMap;

		while (true) {

			indicesAreLeft = currentIndex + 1 <= inIndices.size();
			if (!indicesAreLeft) {
				break;
			}

			bool enoughSpaceForIndices = meshlet.indexCount + 3 < maxIndicesPerMeshlet;
			if (!enoughSpaceForIndices) {
				break;
			}

			size_t vertexCountToAdd = 0;
			for (int i = 0; i < 3; i++) {
				const uint32_t globalIndex = inIndices[currentIndex + i];
				const bool containsVertex  = globalToLocalIndexMap.find(globalIndex) != globalToLocalIndexMap.end();
				if (!containsVertex) {
					vertexCountToAdd++;
				}
			}

			bool enoughSpaceForVertices = meshlet.vertexCount + vertexCountToAdd < maxVerticesPerMeshlet;
			if (!enoughSpaceForVertices) {
				break;
			}

			for (int i = 0; i < 3; i++) {
				const uint32_t globalIndex = inIndices[currentIndex + i];

				uint32_t localIndex;
				const bool indexAlreadyExists = globalToLocalIndexMap.find(globalIndex) != globalToLocalIndexMap.end();
				if (indexAlreadyExists) {
					localIndex = globalToLocalIndexMap[globalIndex];
				}
				else {
					localIndex = globalToLocalIndexMap.size();
					globalToLocalIndexMap[globalIndex] = localIndex;
				}

				data.localIndices.push_back(localIndex);
			}

			meshlet.indexCount  += 3;
			currentIndex        += 3;
			meshlet.vertexCount += vertexCountToAdd;
		}

		for (const auto& iterator : globalToLocalIndexMap) {
			const uint32_t globalIndex = iterator.first;
			const uint32_t localIndex  = iterator.second;

			const Vertex v = inVertices[globalIndex];
			data.vertices.push_back(v);
		}

		data.meshlets.push_back(meshlet);
	}

	return data;
}

std::vector<uint32_t> assetLoaderIndicesTo32BitIndices(const std::vector<uint8_t>& indexData, vkcv::asset::IndexType indexType) {
	std::vector<uint32_t> indices;
	if (indexType == vkcv::asset::IndexType::UINT16) {
		for (int i = 0; i < indexData.size(); i += 2) {
			const uint16_t index16Bit = *reinterpret_cast<const uint16_t*>(&(indexData[i]));
			const uint32_t index32Bit = static_cast<uint32_t>(index16Bit);
			indices.push_back(index32Bit);
		}
	}
	else if (indexType == vkcv::asset::IndexType::UINT32) {
		for (int i = 0; i < indexData.size(); i += 4) {
			const uint32_t index32Bit = *reinterpret_cast<const uint32_t*>(&(indexData[i]));
			indices.push_back(index32Bit);
		}
	}
	else {
		vkcv_log(vkcv::LogLevel::ERROR, "Unsupported index type");
	}
	return indices;
}

int main(int argc, const char** argv) {
	const char* applicationName = "Mesh shader";

	const int windowWidth = 1280;
	const int windowHeight = 720;
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
		{ "VK_KHR_swapchain", VK_NV_MESH_SHADER_EXTENSION_NAME }
	);

    vkcv::gui::GUI gui (core, window);

    const auto& context = core.getContext();
    const vk::Instance& instance = context.getInstance();
    const vk::PhysicalDevice& physicalDevice = context.getPhysicalDevice();
    const vk::Device& device = context.getDevice();

    vkcv::asset::Scene mesh;
    const char* path = argc > 1 ? argv[1] : "resources/Bunny/Bunny.glb";
    vkcv::asset::loadScene(path, mesh);

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

	// format data for mesh shader
	auto& attributes = mesh.vertexGroups[0].vertexBuffer.attributes;

	std::sort(attributes.begin(), attributes.end(), [](const vkcv::asset::VertexAttribute& x, const vkcv::asset::VertexAttribute& y) {
		return static_cast<uint32_t>(x.type) < static_cast<uint32_t>(y.type);
	});

	const std::vector<vkcv::VertexBufferBinding> vertexBufferBindings = {
			vkcv::VertexBufferBinding(static_cast<vk::DeviceSize>(attributes[0].offset), vertexBuffer.getVulkanHandle()),
			vkcv::VertexBufferBinding(static_cast<vk::DeviceSize>(attributes[1].offset), vertexBuffer.getVulkanHandle()),
			vkcv::VertexBufferBinding(static_cast<vk::DeviceSize>(attributes[2].offset), vertexBuffer.getVulkanHandle()) };

	const auto& bunny = mesh.vertexGroups[0];
	const std::vector<Vertex> interleavedVertices = convertToVertices(bunny.vertexBuffer.data, bunny.numVertices, attributes[0], attributes[1]);

	// mesh shader buffers
	const auto& assetLoaderIndexBuffer              = mesh.vertexGroups[0].indexBuffer;
	const std::vector<uint32_t> indexBuffer32Bit    = assetLoaderIndicesTo32BitIndices(assetLoaderIndexBuffer.data, assetLoaderIndexBuffer.type);
	const auto meshShaderModelData                  = createMeshShaderModelData(interleavedVertices, indexBuffer32Bit);

	auto meshShaderVertexBuffer = core.createBuffer<Vertex>(
		vkcv::BufferType::STORAGE,
		meshShaderModelData.vertices.size());
	meshShaderVertexBuffer.fill(meshShaderModelData.vertices);

	auto meshShaderIndexBuffer = core.createBuffer<uint32_t>(
		vkcv::BufferType::STORAGE,
		meshShaderModelData.localIndices.size());
	meshShaderIndexBuffer.fill(meshShaderModelData.localIndices);

	auto meshletBuffer = core.createBuffer<Meshlet>(
		vkcv::BufferType::STORAGE,
		meshShaderModelData.meshlets.size(),
		vkcv::BufferMemoryType::DEVICE_LOCAL
		);
	meshletBuffer.fill(meshShaderModelData.meshlets);

	// attachments
	const vkcv::AttachmentDescription present_color_attachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		core.getSwapchain().getFormat());

    const vkcv::AttachmentDescription depth_attachment(
            vkcv::AttachmentOperation::STORE,
            vkcv::AttachmentOperation::CLEAR,
            vk::Format::eD32Sfloat
    );

	vkcv::PassConfig bunnyPassDefinition({ present_color_attachment, depth_attachment });
	vkcv::PassHandle renderPass = core.createPass(bunnyPassDefinition);

	if (!renderPass)
	{
		std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	vkcv::ShaderProgram bunnyShaderProgram{};
	vkcv::shader::GLSLCompiler compiler;
	
	compiler.compile(vkcv::ShaderStage::VERTEX, std::filesystem::path("resources/shaders/shader.vert"),
					 [&bunnyShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		 bunnyShaderProgram.addShader(shaderStage, path);
	});
	
	compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("resources/shaders/shader.frag"),
					 [&bunnyShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		bunnyShaderProgram.addShader(shaderStage, path);
	});

    const std::vector<vkcv::VertexAttachment> vertexAttachments = bunnyShaderProgram.getVertexAttachments();
    std::vector<vkcv::VertexBinding> bindings;
    for (size_t i = 0; i < vertexAttachments.size(); i++) {
        bindings.push_back(vkcv::VertexBinding(i, { vertexAttachments[i] }));
    }
    const vkcv::VertexLayout bunnyLayout (bindings);

	const vkcv::PipelineConfig bunnyPipelineDefinition {
            bunnyShaderProgram,
            (uint32_t)windowWidth,
            (uint32_t)windowHeight,
            renderPass,
            { bunnyLayout },
            {},
            false
	};

	vkcv::PipelineHandle bunnyPipeline = core.createGraphicsPipeline(bunnyPipelineDefinition);

	if (!bunnyPipeline)
	{
		std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	// mesh shader
	vkcv::ShaderProgram meshShaderProgram;
	compiler.compile(vkcv::ShaderStage::TASK, std::filesystem::path("resources/shaders/shader.task"),
		[&meshShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		meshShaderProgram.addShader(shaderStage, path);
	});

	compiler.compile(vkcv::ShaderStage::MESH, std::filesystem::path("resources/shaders/shader.mesh"),
		[&meshShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		meshShaderProgram.addShader(shaderStage, path);
	});

	compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("resources/shaders/shader.frag"),
		[&meshShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		meshShaderProgram.addShader(shaderStage, path);
	});

    uint32_t setID = 0;
    vkcv::DescriptorSetHandle meshShaderDescriptorSet = core.createDescriptorSet( meshShaderProgram.getReflectedDescriptors()[setID]);
    const vkcv::VertexLayout meshShaderLayout(bindings);

	const vkcv::PipelineConfig meshShaderPipelineDefinition{
		meshShaderProgram,
		(uint32_t)windowWidth,
		(uint32_t)windowHeight,
		renderPass,
		{meshShaderLayout},
		{core.getDescriptorSet(meshShaderDescriptorSet).layout},
		false
	};

	vkcv::PipelineHandle meshShaderPipeline = core.createGraphicsPipeline(meshShaderPipelineDefinition);

	if (!meshShaderPipeline)
	{
		std::cout << "Error. Could not create mesh shader pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

    vkcv::DescriptorWrites meshShaderWrites;
	meshShaderWrites.storageBufferWrites = {
		vkcv::StorageBufferDescriptorWrite(0, meshShaderVertexBuffer.getHandle()), 
		vkcv::StorageBufferDescriptorWrite(1, meshShaderIndexBuffer.getHandle()),
		vkcv::StorageBufferDescriptorWrite(2, meshletBuffer.getHandle()) };

    core.writeDescriptorSet( meshShaderDescriptorSet, meshShaderWrites);

    vkcv::ImageHandle depthBuffer = core.createImage(vk::Format::eD32Sfloat, windowWidth, windowHeight, 1, false).getHandle();

    auto start = std::chrono::system_clock::now();

	vkcv::ImageHandle swapchainImageHandle = vkcv::ImageHandle::createSwapchainImageHandle();

    const vkcv::Mesh renderMesh(vertexBufferBindings, indexBuffer.getVulkanHandle(), mesh.vertexGroups[0].numIndices, vkcv::IndexBitCount::Bit32);

	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

    vkcv::camera::CameraManager cameraManager(window);
    uint32_t camIndex0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	
	cameraManager.getCamera(camIndex0).setPosition(glm::vec3(0, 0, -2));

	while (window.isWindowOpen())
	{
        window.pollEvents();

		uint32_t swapchainWidth, swapchainHeight; // No resizing = No problem
		if (!core.beginFrame(swapchainWidth, swapchainHeight)) {
			continue;
		}
		
        auto end = std::chrono::system_clock::now();
        auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        start = end;
		
		cameraManager.update(0.000001 * static_cast<double>(deltatime.count()));

		glm::mat4 modelMatrix = *reinterpret_cast<glm::mat4*>(&mesh.meshes.front().modelMatrix);
        glm::mat4 mvp = cameraManager.getActiveCamera().getMVP() * modelMatrix;

        const std::vector<vkcv::ImageHandle> renderTargets = { swapchainInput, depthBuffer };
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		const bool useMeshShader = true;

		vkcv::PushConstants pushConstantData(sizeof(glm::mat4));
		pushConstantData.appendDrawcall(mvp);

		if (useMeshShader) {

			vkcv::DescriptorSetUsage descriptorUsage(0, core.getDescriptorSet(meshShaderDescriptorSet).vulkanHandle);

			core.recordMeshShaderDrawcalls(
				cmdStream,
				renderPass,
				meshShaderPipeline,
				pushConstantData,
				{ vkcv::MeshShaderDrawcall({descriptorUsage}, meshShaderModelData.meshlets.size()) },
				{ renderTargets });
		}
		else {

			core.recordDrawcallsToCmdStream(
				cmdStream,
				renderPass,
				bunnyPipeline,
				pushConstantData,
				{ vkcv::DrawcallInfo(renderMesh, {}) },
				{ renderTargets });
		}

		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
		
		// gui.beginGUI();
		// 
		// ImGui::Begin("Settings");
		// ImGui::End();
		// 
		// gui.endGUI();
	    
	    core.endFrame();
	}
	return 0;
}

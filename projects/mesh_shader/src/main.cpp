#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>

#include <vkcv/shader/GLSLCompiler.hpp>
#include <vkcv/gui/GUI.hpp>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/meshlet/Meshlet.hpp>
//#include <vkcv/meshlet/Tipsify.hpp>

struct Plane {
	glm::vec3 pointOnPlane;
	float padding0;
	glm::vec3 normal;
	float padding1;
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

	glm::vec3 up    = glm::dot(forward, glm::vec3(0, -1, 0)) < 0.99 ? glm::vec3(0, -1, 0) : glm::vec3(1, 0, 0);
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
	cameraPlanes.planes[0].pointOnPlane  = nearCenter;
	cameraPlanes.planes[0].normal           = -forward;
	// far
	cameraPlanes.planes[1].pointOnPlane  = farCenter;
	cameraPlanes.planes[1].normal           = forward;

	// top
	cameraPlanes.planes[2].pointOnPlane  = nearUpCenter;
	cameraPlanes.planes[2].normal           = glm::normalize(glm::cross(farUpCenter - nearUpCenter, right));
	// bot
	cameraPlanes.planes[3].pointOnPlane  = nearDownCenter;
	cameraPlanes.planes[3].normal           = glm::normalize(glm::cross(right, farDownCenter - nearDownCenter));

	// right
	cameraPlanes.planes[4].pointOnPlane  = nearRightCenter;
	cameraPlanes.planes[4].normal           = glm::normalize(glm::cross(up, farRightCenter - nearRightCenter));
	// left
	cameraPlanes.planes[5].pointOnPlane  = nearLeftCenter;
	cameraPlanes.planes[5].normal           = glm::normalize(glm::cross(farLeftCenter - nearLeftCenter, up));

	return cameraPlanes;
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
	std::vector<vkcv::meshlet::Vertex> interleavedVertices = vkcv::meshlet::convertToVertices(bunny.vertexBuffer.data, bunny.numVertices, attributes[0], attributes[1]);
	// mesh shader buffers
	const auto& assetLoaderIndexBuffer              = mesh.vertexGroups[0].indexBuffer;
	std::vector<uint32_t> indexBuffer32Bit    = vkcv::meshlet::assetLoaderIndicesTo32BitIndices(assetLoaderIndexBuffer.data, assetLoaderIndexBuffer.type);
//    std::vector<uint32_t> reorderedIndexBuffer32Bit  = vkcv::meshlet::tipsifyMesh(indexBuffer32Bit, interleavedVertices.size(), 2);

    const auto meshShaderModelData = createMeshShaderModelData(interleavedVertices, indexBuffer32Bit);

	auto meshShaderVertexBuffer = core.createBuffer<vkcv::meshlet::Vertex>(
		vkcv::BufferType::STORAGE,
		meshShaderModelData.vertices.size());
	meshShaderVertexBuffer.fill(meshShaderModelData.vertices);

	auto meshShaderIndexBuffer = core.createBuffer<uint32_t>(
		vkcv::BufferType::STORAGE,
		meshShaderModelData.localIndices.size());
	meshShaderIndexBuffer.fill(meshShaderModelData.localIndices);

	auto meshletBuffer = core.createBuffer<vkcv::meshlet::Meshlet>(
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

	vkcv::Buffer<CameraPlanes> cameraPlaneBuffer = core.createBuffer<CameraPlanes>(vkcv::BufferType::UNIFORM, 1);

	struct ObjectMatrices {
		glm::mat4 model;
		glm::mat4 mvp;
	};
	const size_t objectCount = 1;
	vkcv::Buffer<ObjectMatrices> matrixBuffer = core.createBuffer<ObjectMatrices>(vkcv::BufferType::STORAGE, objectCount);

    vkcv::DescriptorWrites meshShaderWrites;
	meshShaderWrites.storageBufferWrites = {
		vkcv::StorageBufferDescriptorWrite(0, meshShaderVertexBuffer.getHandle()),
		vkcv::StorageBufferDescriptorWrite(1, meshShaderIndexBuffer.getHandle()),
		vkcv::StorageBufferDescriptorWrite(2, meshletBuffer.getHandle()),
		vkcv::StorageBufferDescriptorWrite(4, matrixBuffer.getHandle()) 
	};
	meshShaderWrites.uniformBufferWrites = {
		vkcv::UniformBufferDescriptorWrite(3, cameraPlaneBuffer.getHandle())
	};

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
		vkcv::Window::pollEvents();

		uint32_t swapchainWidth, swapchainHeight; // No resizing = No problem
		if (!core.beginFrame(swapchainWidth, swapchainHeight)) {
			continue;
		}
		
		auto end = std::chrono::system_clock::now();
		auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		start = end;
		
		cameraManager.update(0.000001 * static_cast<double>(deltatime.count()));

		const vkcv::camera::Camera& camera = cameraManager.getActiveCamera();

		ObjectMatrices objectMatrices;
		objectMatrices.model    = *reinterpret_cast<glm::mat4*>(&mesh.meshes.front().modelMatrix);
		objectMatrices.mvp      = camera.getMVP() * objectMatrices.model;

		matrixBuffer.fill({ objectMatrices });

		struct PushConstants {
			uint32_t meshletCount;
			uint32_t matrixIndex;
		};
		PushConstants pushConstants{ meshShaderModelData.meshlets.size(), 0 };

		const CameraPlanes cameraPlanes = computeCameraPlanes(camera);
		cameraPlaneBuffer.fill({ cameraPlanes });

		const std::vector<vkcv::ImageHandle> renderTargets = { swapchainInput, depthBuffer };
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		const bool useMeshShader = true;

		vkcv::PushConstants pushConstantData(sizeof(pushConstants));
		pushConstantData.appendDrawcall(pushConstants);

		if (useMeshShader) {

			vkcv::DescriptorSetUsage descriptorUsage(0, core.getDescriptorSet(meshShaderDescriptorSet).vulkanHandle);
			const uint32_t taskCount = (meshShaderModelData.meshlets.size() + 31) / 32;

			core.recordMeshShaderDrawcalls(
				cmdStream,
				renderPass,
				meshShaderPipeline,
				pushConstantData,
				{ vkcv::MeshShaderDrawcall({descriptorUsage}, taskCount)},
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

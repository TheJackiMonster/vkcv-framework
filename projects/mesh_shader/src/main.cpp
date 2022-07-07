#include <iostream>
#include <vkcv/Buffer.hpp>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>

#include <vkcv/shader/GLSLCompiler.hpp>
#include <vkcv/gui/GUI.hpp>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/meshlet/Meshlet.hpp>
#include <vkcv/meshlet/Tipsify.hpp>
#include <vkcv/meshlet/Forsyth.hpp>

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

int main(int argc, const char** argv) {
	const char* applicationName = "Mesh shader";
	
	vkcv::Features features;
	features.requireExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	features.requireExtensionFeature<vk::PhysicalDeviceMeshShaderFeaturesNV>(
			VK_NV_MESH_SHADER_EXTENSION_NAME, [](vk::PhysicalDeviceMeshShaderFeaturesNV& features) {
		features.setTaskShader(true);
		features.setMeshShader(true);
	});

	vkcv::Core core = vkcv::Core::create(
		applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		{ vk::QueueFlagBits::eTransfer,vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute },
		features
	);
	vkcv::WindowHandle windowHandle = core.createWindow(applicationName, 1280, 720, true);
	vkcv::Window &window = core.getWindow(windowHandle);

    vkcv::gui::GUI gui (core, windowHandle);

    vkcv::asset::Scene mesh;
    const char* path = argc > 1 ? argv[1] : "assets/Bunny/Bunny.glb";
    vkcv::asset::loadScene(path, mesh);

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
	const auto& assetLoaderIndexBuffer                    = mesh.vertexGroups[0].indexBuffer;
	std::vector<uint32_t> indexBuffer32Bit                = vkcv::meshlet::assetLoaderIndicesTo32BitIndices(assetLoaderIndexBuffer.data, assetLoaderIndexBuffer.type);
    vkcv::meshlet::VertexCacheReorderResult tipsifyResult = vkcv::meshlet::tipsifyMesh(indexBuffer32Bit, interleavedVertices.size());
    vkcv::meshlet::VertexCacheReorderResult forsythResult = vkcv::meshlet::forsythReorder(indexBuffer32Bit, interleavedVertices.size());

    const auto meshShaderModelData = createMeshShaderModelData(interleavedVertices, forsythResult.indexBuffer, forsythResult.skippedIndices);

	auto meshShaderVertexBuffer = vkcv::buffer<vkcv::meshlet::Vertex>(
		core,
		vkcv::BufferType::STORAGE,
		meshShaderModelData.vertices.size());
	meshShaderVertexBuffer.fill(meshShaderModelData.vertices);

	auto meshShaderIndexBuffer = vkcv::buffer<uint32_t>(
		core,
		vkcv::BufferType::STORAGE,
		meshShaderModelData.localIndices.size());
	meshShaderIndexBuffer.fill(meshShaderModelData.localIndices);

	auto meshletBuffer = vkcv::buffer<vkcv::meshlet::Meshlet>(
		core,
		vkcv::BufferType::STORAGE,
		meshShaderModelData.meshlets.size(),
		vkcv::BufferMemoryType::DEVICE_LOCAL
		);
	meshletBuffer.fill(meshShaderModelData.meshlets);

	// attachments
	const vkcv::AttachmentDescription present_color_attachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		core.getSwapchain(windowHandle).getFormat());

    const vkcv::AttachmentDescription depth_attachment(
            vkcv::AttachmentOperation::STORE,
            vkcv::AttachmentOperation::CLEAR,
            vk::Format::eD32Sfloat
    );

	vkcv::PassConfig bunnyPassDefinition(
			{ present_color_attachment, depth_attachment },
			vkcv::Multisampling::None
	);
	
	vkcv::PassHandle renderPass = core.createPass(bunnyPassDefinition);

	if (!renderPass)
	{
		std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	vkcv::ShaderProgram bunnyShaderProgram{};
	vkcv::shader::GLSLCompiler compiler;
	
	compiler.compile(vkcv::ShaderStage::VERTEX, std::filesystem::path("assets/shaders/shader.vert"),
					 [&bunnyShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		 bunnyShaderProgram.addShader(shaderStage, path);
	});
	
	compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("assets/shaders/shader.frag"),
					 [&bunnyShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		bunnyShaderProgram.addShader(shaderStage, path);
	});

    const std::vector<vkcv::VertexAttachment> vertexAttachments = bunnyShaderProgram.getVertexAttachments();
    std::vector<vkcv::VertexBinding> bindings;
    for (size_t i = 0; i < vertexAttachments.size(); i++) {
        bindings.push_back(vkcv::createVertexBinding(i, { vertexAttachments[i] }));
    }
    const vkcv::VertexLayout bunnyLayout { bindings };

    vkcv::DescriptorSetLayoutHandle vertexShaderDescriptorSetLayout = core.createDescriptorSetLayout(bunnyShaderProgram.getReflectedDescriptors().at(0));
    vkcv::DescriptorSetHandle vertexShaderDescriptorSet = core.createDescriptorSet(vertexShaderDescriptorSetLayout);
	
	const vkcv::GraphicsPipelineConfig bunnyPipelineDefinition {
			bunnyShaderProgram,
			UINT32_MAX,
			UINT32_MAX,
			renderPass,
			{ bunnyLayout },
			{ vertexShaderDescriptorSetLayout },
			true
	};

	struct ObjectMatrices {
		glm::mat4 model;
		glm::mat4 mvp;
	};
	const size_t objectCount = 1;
	vkcv::Buffer<ObjectMatrices> matrixBuffer = vkcv::buffer<ObjectMatrices>(core, vkcv::BufferType::STORAGE, objectCount);

	vkcv::DescriptorWrites vertexShaderDescriptorWrites;
	vertexShaderDescriptorWrites.writeStorageBuffer(0, matrixBuffer.getHandle());
	core.writeDescriptorSet(vertexShaderDescriptorSet, vertexShaderDescriptorWrites);

	vkcv::GraphicsPipelineHandle bunnyPipeline = core.createGraphicsPipeline(bunnyPipelineDefinition);

	if (!bunnyPipeline)
	{
		std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	// mesh shader
	vkcv::ShaderProgram meshShaderProgram;
	compiler.compile(vkcv::ShaderStage::TASK, std::filesystem::path("assets/shaders/shader.task"),
		[&meshShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		meshShaderProgram.addShader(shaderStage, path);
	});

	compiler.compile(vkcv::ShaderStage::MESH, std::filesystem::path("assets/shaders/shader.mesh"),
		[&meshShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		meshShaderProgram.addShader(shaderStage, path);
	});

	compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("assets/shaders/shader.frag"),
		[&meshShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		meshShaderProgram.addShader(shaderStage, path);
	});

	vkcv::DescriptorSetLayoutHandle meshShaderDescriptorSetLayout = core.createDescriptorSetLayout(meshShaderProgram.getReflectedDescriptors().at(0));
	vkcv::DescriptorSetHandle meshShaderDescriptorSet = core.createDescriptorSet(meshShaderDescriptorSetLayout);
	const vkcv::VertexLayout meshShaderLayout(bindings);

	const vkcv::GraphicsPipelineConfig meshShaderPipelineDefinition{
		meshShaderProgram,
		UINT32_MAX,
		UINT32_MAX,
		renderPass,
		{meshShaderLayout},
		{meshShaderDescriptorSetLayout},
		true
	};

	vkcv::GraphicsPipelineHandle meshShaderPipeline = core.createGraphicsPipeline(meshShaderPipelineDefinition);

	if (!meshShaderPipeline)
	{
		std::cout << "Error. Could not create mesh shader pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	vkcv::Buffer<CameraPlanes> cameraPlaneBuffer = vkcv::buffer<CameraPlanes>(core, vkcv::BufferType::UNIFORM, 1);

	vkcv::DescriptorWrites meshShaderWrites;
	meshShaderWrites.writeStorageBuffer(
			0, meshShaderVertexBuffer.getHandle()
	).writeStorageBuffer(
			1, meshShaderIndexBuffer.getHandle()
	).writeStorageBuffer(
			2, meshletBuffer.getHandle()
	).writeStorageBuffer(
			4, matrixBuffer.getHandle()
	).writeStorageBuffer(
			5, meshletBuffer.getHandle()
	);
	
	meshShaderWrites.writeUniformBuffer(3, cameraPlaneBuffer.getHandle());

    core.writeDescriptorSet( meshShaderDescriptorSet, meshShaderWrites);

    vkcv::ImageHandle depthBuffer;

    auto start = std::chrono::system_clock::now();

	vkcv::ImageHandle swapchainImageHandle = vkcv::ImageHandle::createSwapchainImageHandle();

    const vkcv::Mesh renderMesh(vertexBufferBindings, indexBuffer.getVulkanHandle(), mesh.vertexGroups[0].numIndices, vkcv::IndexBitCount::Bit32);

	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

	vkcv::camera::CameraManager cameraManager(window);
	uint32_t camIndex0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	
	cameraManager.getCamera(camIndex0).setPosition(glm::vec3(0, 0, -2));

	bool useMeshShader          = true;
	bool updateFrustumPlanes    = true;

	while (vkcv::Window::hasOpenWindow())
	{
		vkcv::Window::pollEvents();

		uint32_t swapchainWidth, swapchainHeight; // No resizing = No problem
		if (!core.beginFrame(swapchainWidth, swapchainHeight,windowHandle)) {
			continue;
		}
		
		if ((!depthBuffer) ||
			(swapchainWidth != core.getImageWidth(depthBuffer)) ||
			(swapchainHeight != core.getImageHeight(depthBuffer))) {
			depthBuffer = core.createImage(
					vk::Format::eD32Sfloat,
					swapchainWidth,
					swapchainHeight
			).getHandle();
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
			uint32_t matrixIndex;
			uint32_t meshletCount;
		};
		PushConstants pushConstants{ 0, static_cast<uint32_t>(meshShaderModelData.meshlets.size()) };

		if (updateFrustumPlanes) {
			const CameraPlanes cameraPlanes = computeCameraPlanes(camera);
			cameraPlaneBuffer.fill({ cameraPlanes });
		}

		const std::vector<vkcv::ImageHandle> renderTargets = { swapchainInput, depthBuffer };
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		vkcv::PushConstants pushConstantData = vkcv::pushConstants<PushConstants>();
		pushConstantData.appendDrawcall(pushConstants);

		if (useMeshShader) {

			vkcv::DescriptorSetUsage descriptorUsage(0, meshShaderDescriptorSet);
			const uint32_t taskCount = (meshShaderModelData.meshlets.size() + 31) / 32;

			core.recordMeshShaderDrawcalls(
				cmdStream,
				renderPass,
				meshShaderPipeline,
				pushConstantData,
				{ vkcv::MeshShaderDrawcall({descriptorUsage}, taskCount)},
				{ renderTargets },
				windowHandle);
		}
		else {

			vkcv::DescriptorSetUsage descriptorUsage(0, vertexShaderDescriptorSet);

			core.recordDrawcallsToCmdStream(
				cmdStream,
				renderPass,
				bunnyPipeline,
				pushConstantData,
				{ vkcv::DrawcallInfo(renderMesh, { descriptorUsage }) },
				{ renderTargets },
				windowHandle);
		}

		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
		
		gui.beginGUI();
		
		ImGui::Begin("Settings");
		ImGui::Checkbox("Use mesh shader", &useMeshShader);
		ImGui::Checkbox("Update frustum culling", &updateFrustumPlanes);

		ImGui::End();
		
		gui.endGUI();

		core.endFrame(windowHandle);
	}
	return 0;
}

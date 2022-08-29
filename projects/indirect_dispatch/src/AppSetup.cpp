#include "AppSetup.hpp"
#include "AppConfig.hpp"
#include <vkcv/Buffer.hpp>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>

bool loadMesh(vkcv::Core& core, const std::filesystem::path& path, MeshResources* outMesh) {
	assert(outMesh);

	vkcv::asset::Scene scene;
	const int meshLoadResult = vkcv::asset::loadScene(path.string(), scene);

	if (meshLoadResult != 1) {
		vkcv_log(vkcv::LogLevel::ERROR, "Mesh loading failed");
		return false;
	}

	if (scene.meshes.size() < 1) {
		vkcv_log(vkcv::LogLevel::ERROR, "Cube mesh scene does not contain any vertex groups");
		return false;
	}
	assert(!scene.vertexGroups.empty());

	auto& vertexData = scene.vertexGroups[0].vertexBuffer;
	auto& indexData  = scene.vertexGroups[0].indexBuffer;

	vkcv::Buffer<uint8_t> vertexBuffer = vkcv::buffer<uint8_t>(
		core,
		vkcv::BufferType::VERTEX,
		vertexData.data.size(),
		vkcv::BufferMemoryType::DEVICE_LOCAL);

	vkcv::Buffer<uint8_t> indexBuffer = vkcv::buffer<uint8_t>(
		core,
		vkcv::BufferType::INDEX,
		indexData.data.size(),
		vkcv::BufferMemoryType::DEVICE_LOCAL);

	vertexBuffer.fill(vertexData.data);
	indexBuffer.fill(indexData.data);

	outMesh->vertexBuffer = vertexBuffer.getHandle();
	outMesh->indexBuffer  = indexBuffer.getHandle();

	auto& attributes = vertexData.attributes;

	std::sort(attributes.begin(), attributes.end(),
		[](const vkcv::asset::VertexAttribute& x, const vkcv::asset::VertexAttribute& y) {
		return static_cast<uint32_t>(x.type) < static_cast<uint32_t>(y.type);
	});

	const std::vector<vkcv::VertexBufferBinding> vertexBufferBindings = {
		vkcv::VertexBufferBinding(static_cast<vk::DeviceSize>(attributes[0].offset), vertexBuffer.getVulkanHandle()),
		vkcv::VertexBufferBinding(static_cast<vk::DeviceSize>(attributes[1].offset), vertexBuffer.getVulkanHandle()),
		vkcv::VertexBufferBinding(static_cast<vk::DeviceSize>(attributes[2].offset), vertexBuffer.getVulkanHandle()) };

	outMesh->mesh = vkcv::Mesh(vertexBufferBindings, indexBuffer.getVulkanHandle(), scene.vertexGroups[0].numIndices);

	return true;
}

bool loadImage(vkcv::Core& core, const std::filesystem::path& path, vkcv::ImageHandle* outImage) {

	assert(outImage);

	const vkcv::asset::Texture textureData = vkcv::asset::loadTexture(path);

	if (textureData.channels != 4) {
		vkcv_log(vkcv::LogLevel::ERROR, "Expecting image with four components");
		return false;
	}

	vkcv::Image image = core.createImage(
		vk::Format::eR8G8B8A8Srgb, 
		textureData.width, 
		textureData.height, 
		1, 
		true);

	image.fill(textureData.data.data(), textureData.data.size());
	
	{
		auto mipStream = core.createCommandStream(vkcv::QueueType::Graphics);
		image.recordMipChainGeneration(mipStream, core.getDownsampler());
		core.submitCommandStream(mipStream, false);
	}

	*outImage = image.getHandle();
	return true;
}

bool loadGraphicPass(
	vkcv::Core& core,
	const std::filesystem::path vertexPath,
	const std::filesystem::path fragmentPath,
	const vkcv::PassConfig&     passConfig,
	const vkcv::DepthTest       depthTest,
	GraphicPassHandles*         outPassHandles) {

	assert(outPassHandles);

	outPassHandles->renderPass = core.createPass(passConfig);

	if (!outPassHandles->renderPass) {
		vkcv_log(vkcv::LogLevel::ERROR, "Error: Could not create renderpass");
		return false;
	}

	vkcv::ShaderProgram         shaderProgram;
	vkcv::shader::GLSLCompiler  compiler;

	compiler.compile(vkcv::ShaderStage::VERTEX, vertexPath,
		[&shaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shaderProgram.addShader(shaderStage, path);
	});

	compiler.compile(vkcv::ShaderStage::FRAGMENT, fragmentPath,
		[&shaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shaderProgram.addShader(shaderStage, path);
	});

	const std::vector<vkcv::VertexAttachment> vertexAttachments = shaderProgram.getVertexAttachments();
	std::vector<vkcv::VertexBinding> bindings;
	for (size_t i = 0; i < vertexAttachments.size(); i++) {
		bindings.push_back(vkcv::createVertexBinding(i, { vertexAttachments[i] }));
	}

	const vkcv::VertexLayout vertexLayout { bindings };

	const auto descriptorBindings = shaderProgram.getReflectedDescriptors();
	const bool hasDescriptor = descriptorBindings.size() > 0;
	std::vector<vkcv::DescriptorSetLayoutHandle> descriptorSetLayouts = {};
	if (hasDescriptor)
	{
	    outPassHandles->descriptorSetLayout = core.createDescriptorSetLayout(descriptorBindings.at(0));
	    outPassHandles->descriptorSet = core.createDescriptorSet(outPassHandles->descriptorSetLayout);
	    descriptorSetLayouts.push_back(outPassHandles->descriptorSetLayout);
	}
	
	vkcv::GraphicsPipelineConfig pipelineConfig(
		shaderProgram,
		outPassHandles->renderPass,
		{ vertexLayout },
		descriptorSetLayouts
	);
	
	pipelineConfig.setDepthTest(depthTest);
	outPassHandles->pipeline = core.createGraphicsPipeline(pipelineConfig);

	if (!outPassHandles->pipeline) {
		vkcv_log(vkcv::LogLevel::ERROR, "Error: Could not create graphics pipeline");
		return false;
	}

	return true;
}

bool loadMeshPass(vkcv::Core& core, GraphicPassHandles* outHandles) {

	assert(outHandles);

	vkcv::AttachmentDescription colorAttachment(
			AppConfig::colorBufferFormat,
			vkcv::AttachmentOperation::DONT_CARE,
			vkcv::AttachmentOperation::STORE
	);

	vkcv::AttachmentDescription depthAttachment(
			AppConfig::depthBufferFormat,
			vkcv::AttachmentOperation::LOAD,
			vkcv::AttachmentOperation::STORE
	);

	return loadGraphicPass(
		core,
		"assets/shaders/mesh.vert",
		"assets/shaders/mesh.frag",
		vkcv::PassConfig(
				{ colorAttachment, depthAttachment },
				vkcv::Multisampling::None
		),
		vkcv::DepthTest::Equal,
		outHandles);
}

bool loadSkyPass(vkcv::Core& core, GraphicPassHandles* outHandles) {

	assert(outHandles);

	vkcv::AttachmentDescription colorAttachment(
			AppConfig::colorBufferFormat,
			vkcv::AttachmentOperation::LOAD,
			vkcv::AttachmentOperation::STORE
	);

	vkcv::AttachmentDescription depthAttachment(
			AppConfig::depthBufferFormat,
			vkcv::AttachmentOperation::LOAD,
			vkcv::AttachmentOperation::STORE
	);

	return loadGraphicPass(
		core,
		"assets/shaders/sky.vert",
		"assets/shaders/sky.frag",
		vkcv::PassConfig(
				{ colorAttachment, depthAttachment },
				vkcv::Multisampling::None
		),
		vkcv::DepthTest::Equal,
		outHandles);
}

bool loadPrePass(vkcv::Core& core, GraphicPassHandles* outHandles) {
	assert(outHandles);

	vkcv::AttachmentDescription motionAttachment(
			AppConfig::motionBufferFormat,
			vkcv::AttachmentOperation::CLEAR,
			vkcv::AttachmentOperation::STORE
	);

	vkcv::AttachmentDescription depthAttachment(
			AppConfig::depthBufferFormat,
			vkcv::AttachmentOperation::CLEAR,
			vkcv::AttachmentOperation::STORE
	);

	return loadGraphicPass(
		core,
		"assets/shaders/prepass.vert",
		"assets/shaders/prepass.frag",
		vkcv::PassConfig(
				{ motionAttachment, depthAttachment },
				vkcv::Multisampling::None
		),
		vkcv::DepthTest::LessEqual,
		outHandles);
}

bool loadSkyPrePass(vkcv::Core& core, GraphicPassHandles* outHandles) {
	assert(outHandles);

	vkcv::AttachmentDescription motionAttachment(
			AppConfig::motionBufferFormat,
			vkcv::AttachmentOperation::LOAD,
			vkcv::AttachmentOperation::STORE
	);

	vkcv::AttachmentDescription depthAttachment(
			AppConfig::depthBufferFormat,
			vkcv::AttachmentOperation::LOAD,
			vkcv::AttachmentOperation::STORE
	);

	return loadGraphicPass(
		core,
		"assets/shaders/skyPrepass.vert",
		"assets/shaders/skyPrepass.frag",
		vkcv::PassConfig(
				{ motionAttachment, depthAttachment },
				vkcv::Multisampling::None
		),
		vkcv::DepthTest::LessEqual,
		outHandles);
}

bool loadComputePass(vkcv::Core& core, const std::filesystem::path& path, ComputePassHandles* outComputePass) {

	assert(outComputePass);
	vkcv::ShaderProgram shaderProgram;
	vkcv::shader::GLSLCompiler compiler;

	compiler.compile(vkcv::ShaderStage::COMPUTE, path,
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shaderProgram.addShader(shaderStage, path);
	});

	if (shaderProgram.getReflectedDescriptors().size() < 1) {
		vkcv_log(vkcv::LogLevel::ERROR, "Compute shader has no descriptor set");
		return false;
	}

	outComputePass->descriptorSetLayout = core.createDescriptorSetLayout(shaderProgram.getReflectedDescriptors().at(0));
	outComputePass->descriptorSet = core.createDescriptorSet(outComputePass->descriptorSetLayout);
	outComputePass->pipeline = core.createComputePipeline({
		shaderProgram,
		{ outComputePass->descriptorSetLayout }});

	if (!outComputePass->pipeline) {
		vkcv_log(vkcv::LogLevel::ERROR, "Compute shader pipeline creation failed");
		return false;
	}

	return true;
}

AppRenderTargets createRenderTargets(vkcv::Core& core, const uint32_t width, const uint32_t height) {

	AppRenderTargets targets;

	targets.depthBuffer = core.createImage(
		AppConfig::depthBufferFormat,
		width,
		height,
		1,
		false).getHandle();

	targets.colorBuffer = core.createImage(
		AppConfig::colorBufferFormat,
		width,
		height,
		1,
		false,
		false,
		true).getHandle();

	targets.motionBuffer = core.createImage(
		AppConfig::motionBufferFormat,
		width,
		height,
		1,
		false,
		false,
		true).getHandle();

	return targets;
}
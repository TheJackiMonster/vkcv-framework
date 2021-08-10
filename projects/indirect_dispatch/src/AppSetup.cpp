#include "AppSetup.hpp"
#include "AppConfig.hpp"
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

	vkcv::Buffer vertexBuffer = core.createBuffer<uint8_t>(
		vkcv::BufferType::VERTEX,
		vertexData.data.size(),
		vkcv::BufferMemoryType::DEVICE_LOCAL);

	vkcv::Buffer indexBuffer = core.createBuffer<uint8_t>(
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
		vkcv::VertexBufferBinding(static_cast<vk::DeviceSize>(attributes[1].offset), vertexBuffer.getVulkanHandle()) };

	outMesh->mesh = vkcv::Mesh(vertexBufferBindings, indexBuffer.getVulkanHandle(), scene.vertexGroups[0].numIndices);

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
		bindings.push_back(vkcv::VertexBinding(i, { vertexAttachments[i] }));
	}

	const vkcv::VertexLayout vertexLayout(bindings);

	vkcv::PipelineConfig pipelineConfig{
		shaderProgram,
		UINT32_MAX,
		UINT32_MAX,
		outPassHandles->renderPass,
		{ vertexLayout },
		{},
		true };
	pipelineConfig.m_depthTest  = depthTest;
	outPassHandles->pipeline    = core.createGraphicsPipeline(pipelineConfig);

	if (!outPassHandles->pipeline) {
		vkcv_log(vkcv::LogLevel::ERROR, "Error: Could not create graphics pipeline");
		return false;
	}

	return true;
}

bool loadMeshPass(vkcv::Core& core, GraphicPassHandles* outHandles) {

	assert(outHandles);

	vkcv::AttachmentDescription colorAttachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::DONT_CARE,
		AppConfig::colorBufferFormat);

	vkcv::AttachmentDescription depthAttachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::LOAD,
		AppConfig::depthBufferFormat);

	return loadGraphicPass(
		core,
		"resources/shaders/mesh.vert",
		"resources/shaders/mesh.frag",
		vkcv::PassConfig({ colorAttachment, depthAttachment }),
		vkcv::DepthTest::Equal,
		outHandles);
}

bool loadSkyPass(vkcv::Core& core, GraphicPassHandles* outHandles) {

	assert(outHandles);

	vkcv::AttachmentDescription colorAttachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::LOAD,
		AppConfig::colorBufferFormat);

	vkcv::AttachmentDescription depthAttachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::LOAD,
		AppConfig::depthBufferFormat);

	return loadGraphicPass(
		core,
		"resources/shaders/sky.vert",
		"resources/shaders/sky.frag",
		vkcv::PassConfig({ colorAttachment, depthAttachment }),
		vkcv::DepthTest::Equal,
		outHandles);
}

bool loadPrePass(vkcv::Core& core, GraphicPassHandles* outHandles) {
	assert(outHandles);

	vkcv::AttachmentDescription motionAttachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		AppConfig::motionBufferFormat);

	vkcv::AttachmentDescription depthAttachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		AppConfig::depthBufferFormat);

	return loadGraphicPass(
		core,
		"resources/shaders/prepass.vert",
		"resources/shaders/prepass.frag",
		vkcv::PassConfig({ motionAttachment, depthAttachment }),
		vkcv::DepthTest::LessEqual,
		outHandles);
}

bool loadSkyPrePass(vkcv::Core& core, GraphicPassHandles* outHandles) {
	assert(outHandles);

	vkcv::AttachmentDescription motionAttachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::LOAD,
		AppConfig::motionBufferFormat);

	vkcv::AttachmentDescription depthAttachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::LOAD,
		AppConfig::depthBufferFormat);

	return loadGraphicPass(
		core,
		"resources/shaders/skyPrepass.vert",
		"resources/shaders/skyPrepass.frag",
		vkcv::PassConfig({ motionAttachment, depthAttachment }),
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

	outComputePass->descriptorSet = core.createDescriptorSet(shaderProgram.getReflectedDescriptors()[0]);

	outComputePass->pipeline = core.createComputePipeline(
		shaderProgram,
		{ core.getDescriptorSet(outComputePass->descriptorSet).layout });

	if (!outComputePass->pipeline) {
		vkcv_log(vkcv::LogLevel::ERROR, "Compute shader pipeline creation failed");
		return false;
	}

	return true;
}

RenderTargets createRenderTargets(vkcv::Core& core, const uint32_t width, const uint32_t height) {

	RenderTargets targets;

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

	targets.motionBlurOutput = core.createImage(
		AppConfig::colorBufferFormat,
		width,
		height,
		1,
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
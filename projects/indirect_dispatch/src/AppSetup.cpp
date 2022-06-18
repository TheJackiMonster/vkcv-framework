#include "AppSetup.hpp"
#include "AppConfig.hpp"
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>

struct vertex_t {
	float positionU [4];
	float normalV [4];
};

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

	vkcv::Buffer vertexBuffer = core.createBuffer<vertex_t>(
		vkcv::BufferType::STORAGE,
		scene.vertexGroups[0].numVertices,
		vkcv::BufferMemoryType::DEVICE_LOCAL);
	
	std::vector<vertex_t> vertices;
	vertices.reserve(vertexBuffer.getCount());
	
	for (const auto& attribute : scene.vertexGroups[0].vertexBuffer.attributes) {
		if (attribute.componentType != vkcv::asset::ComponentType::FLOAT32) {
			continue;
		}
		
		size_t offset = attribute.offset;
		
		for (size_t i = 0; i < vertexBuffer.getCount(); i++) {
			const auto *data = reinterpret_cast<const float*>(
					scene.vertexGroups[0].vertexBuffer.data.data() + offset
			);
			
			switch (attribute.type) {
				case vkcv::asset::PrimitiveType::POSITION:
					memcpy(vertices[i].positionU, data, sizeof(float) * attribute.componentCount);
					break;
				case vkcv::asset::PrimitiveType::NORMAL:
					memcpy(vertices[i].normalV, data, sizeof(float) * attribute.componentCount);
					break;
				case vkcv::asset::PrimitiveType::TEXCOORD_0:
					if (attribute.componentCount != 2) {
						break;
					}
					
					vertices[i].positionU[3] = data[0];
					vertices[i].normalV[3] = data[1];
					break;
				default:
					break;
			}
			
			offset += attribute.stride;
		}
	}
	
	vertexBuffer.fill(vertices);

	vkcv::Buffer indexBuffer = core.createBuffer<uint8_t>(
		vkcv::BufferType::INDEX,
		scene.vertexGroups[0].indexBuffer.data.size(),
		vkcv::BufferMemoryType::DEVICE_LOCAL);

	indexBuffer.fill(scene.vertexGroups[0].indexBuffer.data);

	outMesh->vertexBuffer = vertexBuffer.getHandle();
	outMesh->indexBuffer  = indexBuffer.getHandle();

	outMesh->mesh = vkcv::Mesh(indexBuffer.getVulkanHandle(), scene.vertexGroups[0].numIndices);
	
	vkcv::DescriptorBindings descriptorBindings;
	descriptorBindings.insert(std::make_pair(0, vkcv::DescriptorBinding {
		0,
		vkcv::DescriptorType::STORAGE_BUFFER,
		1,
		vkcv::ShaderStage::VERTEX,
		false,
		false
	}));
	
	outMesh->descSetLayout = core.createDescriptorSetLayout(descriptorBindings);
	outMesh->descSet = core.createDescriptorSet(outMesh->descSetLayout);
	
	core.writeDescriptorSet(
			outMesh->descSet,
			vkcv::DescriptorWrites().writeStorageBuffer(0, outMesh->vertexBuffer)
	);

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

	const auto descriptorBindings = shaderProgram.getReflectedDescriptors();
	const bool hasDescriptor = descriptorBindings.size() > 0;
	std::vector<vkcv::DescriptorSetLayoutHandle> descriptorSetLayouts = {};
	
	if (hasDescriptor)
	{
		descriptorSetLayouts.reserve(descriptorBindings.size());
		
		for (size_t i = 0; i < descriptorBindings.size(); i++) {
			descriptorSetLayouts.push_back(core.createDescriptorSetLayout(descriptorBindings.at(i)));
		}
		
	    outPassHandles->descriptorSetLayout = descriptorSetLayouts[0];
	    outPassHandles->descriptorSet = core.createDescriptorSet(outPassHandles->descriptorSetLayout);
	}

	vkcv::GraphicsPipelineConfig pipelineConfig{
		shaderProgram,
		UINT32_MAX,
		UINT32_MAX,
		outPassHandles->renderPass,
		descriptorSetLayouts,
		true
	};
	pipelineConfig.m_depthTest  = depthTest;
	outPassHandles->pipeline    = core.createGraphicsPipeline(pipelineConfig);

	if (!outPassHandles->pipeline) {
		vkcv_log(vkcv::LogLevel::ERROR, "Error: Could not create graphics pipeline [%s]",
				 vertexPath.c_str());
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
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::LOAD,
		AppConfig::colorBufferFormat);

	vkcv::AttachmentDescription depthAttachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::LOAD,
		AppConfig::depthBufferFormat);

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
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		AppConfig::motionBufferFormat);

	vkcv::AttachmentDescription depthAttachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		AppConfig::depthBufferFormat);

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
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::LOAD,
		AppConfig::motionBufferFormat);

	vkcv::AttachmentDescription depthAttachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::LOAD,
		AppConfig::depthBufferFormat);

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
#include "ShadowMapping.hpp"
#include <vkcv/shader/GLSLCompiler.hpp>

vkcv::ShaderProgram loadShadowShader() {
	vkcv::ShaderProgram shader;
	vkcv::shader::GLSLCompiler compiler;
	compiler.compile(vkcv::ShaderStage::VERTEX, "resources/shaders/shadow.vert",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shader.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::FRAGMENT, "resources/shaders/shadow.frag",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shader.addShader(shaderStage, path);
	});
	return shader;
}

const vk::Format    shadowMapFormat     = vk::Format::eD16Unorm;
const uint32_t      shadowMapResolution = 1024;

ShadowMapping::ShadowMapping(vkcv::Core* corePtr, const vkcv::VertexLayout& vertexLayout) : 
	m_corePtr(corePtr),
	m_shadowMap(corePtr->createImage(shadowMapFormat, shadowMapResolution, shadowMapResolution)),
	m_lightInfoBuffer(corePtr->createBuffer<LightInfo>(vkcv::BufferType::UNIFORM, sizeof(glm::vec3))){

	vkcv::ShaderProgram shadowShader = loadShadowShader();

	const std::vector<vkcv::AttachmentDescription> shadowAttachments = {
		vkcv::AttachmentDescription(vkcv::AttachmentOperation::STORE, vkcv::AttachmentOperation::CLEAR, shadowMapFormat)
	};
	const vkcv::PassConfig shadowPassConfig(shadowAttachments);
	m_shadowMapPass = corePtr->createPass(shadowPassConfig);
	vkcv::PipelineConfig shadowPipeConfig{
		shadowShader,
		shadowMapResolution,
		shadowMapResolution,
		m_shadowMapPass,
		vertexLayout,
		{},
		false
	};
	shadowPipeConfig.m_EnableDepthClamping = true;
	m_shadowMapPipe = corePtr->createGraphicsPipeline(shadowPipeConfig);

	// shadow map
	m_shadowSampler = corePtr->createSampler(
		vkcv::SamplerFilterType::NEAREST,
		vkcv::SamplerFilterType::NEAREST,
		vkcv::SamplerMipmapMode::NEAREST,
		vkcv::SamplerAddressMode::CLAMP_TO_EDGE
	);
}

void ShadowMapping::recordShadowMapRendering(
	const vkcv::CommandStreamHandle&    cmdStream, 
	const glm::vec2&                    lightAngleRadian,
	const glm::vec3&                    lightColor,
	const float                         lightStrength,
	const std::vector<vkcv::Mesh>&      meshes,
	const std::vector<glm::mat4>&       modelMatrices,
	const vkcv::camera::Camera&         camera) {

	LightInfo lightInfo;
	lightInfo.sunColor      = lightColor;
	lightInfo.sunStrength   = lightStrength;
	lightInfo.direction     = glm::normalize(glm::vec3(
		std::cos(lightAngleRadian.x) * std::cos(lightAngleRadian.y),
		std::sin(lightAngleRadian.x),
		std::cos(lightAngleRadian.x) * std::sin(lightAngleRadian.y)));

	const float shadowProjectionSize = 20.f;
	glm::mat4 projectionLight = glm::ortho(
		-shadowProjectionSize,
		shadowProjectionSize,
		-shadowProjectionSize,
		shadowProjectionSize,
		-shadowProjectionSize,
		shadowProjectionSize);

	glm::mat4 vulkanCorrectionMatrix(1.f);
	vulkanCorrectionMatrix[2][2] = 0.5;
	vulkanCorrectionMatrix[3][2] = 0.5;
	projectionLight = vulkanCorrectionMatrix * projectionLight;

	const glm::mat4 viewLight = glm::lookAt(glm::vec3(0), lightInfo.direction, glm::vec3(0, -1, 0));

	lightInfo.lightMatrix = projectionLight * viewLight;
	m_lightInfoBuffer.fill({ lightInfo });

	std::vector<glm::mat4> mvpLight;
	for (const auto& m : modelMatrices) {
		mvpLight.push_back(lightInfo.lightMatrix * m);
	}
	const vkcv::PushConstantData shadowPushConstantData((void*)mvpLight.data(), sizeof(glm::mat4));

	std::vector<vkcv::DrawcallInfo> drawcalls;
	for (const auto& mesh : meshes) {
		drawcalls.push_back(vkcv::DrawcallInfo(mesh, {}));
	}

	m_corePtr->recordDrawcallsToCmdStream(
		cmdStream,
		m_shadowMapPass,
		m_shadowMapPipe,
		shadowPushConstantData,
		drawcalls,
		{ m_shadowMap.getHandle() });
	m_corePtr->prepareImageForSampling(cmdStream, m_shadowMap.getHandle());
}

vkcv::ImageHandle ShadowMapping::getShadowMap() {
	return m_shadowMap.getHandle();
}

vkcv::SamplerHandle ShadowMapping::getShadowSampler() {
	return m_shadowSampler;
}

vkcv::BufferHandle ShadowMapping::getLightInfoBuffer() {
	return m_lightInfoBuffer.getHandle();
}
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

glm::mat4 computeShadowViewProjectionMatrix(
	const glm::vec3&            lightDirection, 
	const vkcv::camera::Camera& camera, 
	float                       maxShadowDistance,
	const glm::vec3&            voxelVolumeOffset,
	float                       voxelVolumeExtent) {

	const glm::vec3 cameraPos   = camera.getPosition();
	const glm::vec3 forward     = glm::normalize(camera.getFront());
	glm::vec3 up                = glm::normalize(camera.getUp());
	const glm::vec3 right       = glm::normalize(glm::cross(forward, up));
	up = glm::cross(right, forward);

	const float fov         = camera.getFov();
	const float aspectRatio = camera.getRatio();

	float near;
	float far;
	camera.getNearFar(near, far);
	far = std::min(maxShadowDistance, far);

	const glm::vec3 nearCenter  = cameraPos + forward * near;
	const float nearUp          = near * tan(fov * 0.5);
	const float nearRight       = nearUp * aspectRatio;
	
	const glm::vec3 farCenter   = cameraPos + forward * far;
	const float farUp           = far * tan(fov * 0.5);
	const float farRight        = farUp * aspectRatio;

	std::array<glm::vec3, 8> viewFrustumCorners = {
		nearCenter + right * nearRight + nearUp * up,
		nearCenter + right * nearRight - nearUp * up,
		nearCenter - right * nearRight + nearUp * up,
		nearCenter - right * nearRight - nearUp * up,

		farCenter + right * farRight + farUp * up,
		farCenter + right * farRight - farUp * up,
		farCenter - right * farRight + farUp * up,
		farCenter - right * farRight - farUp * up
	};

	std::array<glm::vec3, 8> voxelVolumeCorners = {
		voxelVolumeOffset + voxelVolumeExtent * glm::vec3(1, 1, 1),
		voxelVolumeOffset + voxelVolumeExtent * glm::vec3(1, 1, -1),
		voxelVolumeOffset + voxelVolumeExtent * glm::vec3(1, -1, 1),
		voxelVolumeOffset + voxelVolumeExtent * glm::vec3(1, -1, -1),

		voxelVolumeOffset + voxelVolumeExtent * glm::vec3(-1, 1, 1),
		voxelVolumeOffset + voxelVolumeExtent * glm::vec3(-1, 1, -1),
		voxelVolumeOffset + voxelVolumeExtent * glm::vec3(-1, -1, 1),
		voxelVolumeOffset + voxelVolumeExtent * glm::vec3(-1, -1, -1),
	};

	glm::vec3 minView(std::numeric_limits<float>::max());
	glm::vec3 maxView(std::numeric_limits<float>::lowest());

	const glm::mat4 view = glm::lookAt(glm::vec3(0), -lightDirection, glm::vec3(0, -1, 0));

	auto getMinMaxView = [&](std::array<glm::vec3, 8> points) {
		for (const glm::vec3& p : points) {
			const auto& pView = glm::vec3(view * glm::vec4(p, 1));
			minView = glm::min(minView, pView);
			maxView = glm::max(maxView, pView);
		}
	};

	getMinMaxView(viewFrustumCorners);
	getMinMaxView(voxelVolumeCorners);

	glm::vec3 scale  = glm::vec3(2) / (maxView - minView);
	glm::vec3 offset = -0.5f * (maxView + minView) * scale;

	glm::mat4 crop(1);
	crop[0][0] = scale.x;
	crop[1][1] = scale.y;
	crop[2][2] = scale.z;

	crop[3][0] = offset.x;
	crop[3][1] = offset.y;
	crop[3][2] = offset.z;

	glm::mat4 vulkanCorrectionMatrix(1.f);
	vulkanCorrectionMatrix[2][2] = 0.5;
	vulkanCorrectionMatrix[3][2] = 0.5;

	return vulkanCorrectionMatrix * crop * view;
}

const vk::Format    shadowMapFormat      = vk::Format::eR16G16B16A16Sfloat;
const vk::Format    shadowMapDepthFormat = vk::Format::eD16Unorm;
const uint32_t      shadowMapResolution  = 2048;

ShadowMapping::ShadowMapping(vkcv::Core* corePtr, const vkcv::VertexLayout& vertexLayout) : 
	m_corePtr(corePtr),
	m_shadowMap(corePtr->createImage(shadowMapFormat, shadowMapResolution, shadowMapResolution, 1, false, false, true)),
	m_shadowMapDepth(corePtr->createImage(shadowMapDepthFormat, shadowMapResolution, shadowMapResolution)),
	m_lightInfoBuffer(corePtr->createBuffer<LightInfo>(vkcv::BufferType::UNIFORM, sizeof(glm::vec3))){

	vkcv::ShaderProgram shadowShader = loadShadowShader();

	// descriptor set
	m_shadowDescriptorSet = corePtr->createDescriptorSet(shadowShader.getReflectedDescriptors()[0]);
	vkcv::DescriptorWrites shadowDescriptorWrites;
	shadowDescriptorWrites.uniformBufferWrites = { vkcv::UniformBufferDescriptorWrite(0, m_lightInfoBuffer.getHandle()) };
	corePtr->writeDescriptorSet(m_shadowDescriptorSet, shadowDescriptorWrites);

	// pass
	const std::vector<vkcv::AttachmentDescription> shadowAttachments = {
		vkcv::AttachmentDescription(vkcv::AttachmentOperation::STORE, vkcv::AttachmentOperation::CLEAR, shadowMapFormat),
		vkcv::AttachmentDescription(vkcv::AttachmentOperation::STORE, vkcv::AttachmentOperation::CLEAR, shadowMapDepthFormat)
	};
	const vkcv::PassConfig shadowPassConfig(shadowAttachments);
	m_shadowMapPass = corePtr->createPass(shadowPassConfig);

	// pipeline
	vkcv::PipelineConfig shadowPipeConfig{
		shadowShader,
		shadowMapResolution,
		shadowMapResolution,
		m_shadowMapPass,
		vertexLayout,
		{ corePtr->getDescriptorSet(m_shadowDescriptorSet).layout },
		false
	};
	shadowPipeConfig.m_EnableDepthClamping = true;
	m_shadowMapPipe = corePtr->createGraphicsPipeline(shadowPipeConfig);

	m_shadowSampler = corePtr->createSampler(
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerMipmapMode::NEAREST,
		vkcv::SamplerAddressMode::CLAMP_TO_EDGE
	);
}

void ShadowMapping::recordShadowMapRendering(
	const vkcv::CommandStreamHandle&    cmdStream, 
	const glm::vec2&                    lightAngleRadian,
	const glm::vec3&                    lightColor,
	float                               lightStrength,
	float                               maxShadowDistance,
	float                               exponentialWarpPositive,
	float                               exponentialWarpNegative,
	const std::vector<vkcv::Mesh>&      meshes,
	const std::vector<glm::mat4>&       modelMatrices,
	const vkcv::camera::Camera&         camera,
	const glm::vec3&                    voxelVolumeOffset,
	float                               voxelVolumeExtent) {

	LightInfo lightInfo;
	lightInfo.sunColor      = lightColor;
	lightInfo.sunStrength   = lightStrength;
	lightInfo.direction     = glm::normalize(glm::vec3(
		std::cos(lightAngleRadian.x) * std::cos(lightAngleRadian.y),
		std::sin(lightAngleRadian.x),
		std::cos(lightAngleRadian.x) * std::sin(lightAngleRadian.y)));
	lightInfo.exponentialWarpPositive = exponentialWarpPositive;
	lightInfo.exponentialWarpNegative = exponentialWarpNegative;

	lightInfo.lightMatrix = computeShadowViewProjectionMatrix(
		lightInfo.direction,
		camera,
		maxShadowDistance,
		voxelVolumeOffset,
		voxelVolumeExtent);
	m_lightInfoBuffer.fill({ lightInfo });

	std::vector<glm::mat4> mvpLight;
	for (const auto& m : modelMatrices) {
		mvpLight.push_back(lightInfo.lightMatrix * m);
	}
	const vkcv::PushConstantData shadowPushConstantData((void*)mvpLight.data(), sizeof(glm::mat4));

	std::vector<vkcv::DrawcallInfo> drawcalls;
	for (const auto& mesh : meshes) {
		drawcalls.push_back(vkcv::DrawcallInfo(mesh, { vkcv::DescriptorSetUsage(0, m_corePtr->getDescriptorSet(m_shadowDescriptorSet).vulkanHandle) }));
	}

	m_corePtr->recordDrawcallsToCmdStream(
		cmdStream,
		m_shadowMapPass,
		m_shadowMapPipe,
		shadowPushConstantData,
		drawcalls,
		{ m_shadowMap.getHandle(), m_shadowMapDepth.getHandle() });
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
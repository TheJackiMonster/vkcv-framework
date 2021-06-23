#pragma once
#include <vkcv/core.hpp>
#include <vkcv/camera/Camera.hpp>

#define GLM_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

struct LightInfo {
	glm::vec3   direction;
	float       padding;
	glm::vec3   sunColor;
	float       sunStrength;
	glm::mat4   lightMatrix;
	float       exponentialWarpPositive;
	float       exponentialWarpNegative;
};

class ShadowMapping {
public:
	ShadowMapping(vkcv::Core* corePtr, const vkcv::VertexLayout& vertexLayout);

	void recordShadowMapRendering(
		const vkcv::CommandStreamHandle&    cmdStream,
		const glm::vec2&                    lightAngleRadian,
		const glm::vec3&                    lightColor,
		float                               lightStrength,
		float                               maxShadowDistance,
		float                               exponentialWarp,
		float                               exponentialWarpNegative,
		const std::vector<vkcv::Mesh>&      meshes,
		const std::vector<glm::mat4>&       modelMatrices,
		const vkcv::camera::Camera&         camera,
		const glm::vec3&                    voxelVolumeOffset,
		float                               voxelVolumeExtent);

	vkcv::ImageHandle   getShadowMap();
	vkcv::SamplerHandle getShadowSampler();
	vkcv::BufferHandle  getLightInfoBuffer();

private:
	vkcv::Core* m_corePtr;

	vkcv::Image             m_shadowMap;
	vkcv::Image             m_shadowMapDepth;
	vkcv::SamplerHandle     m_shadowSampler;
	vkcv::Buffer<LightInfo> m_lightInfoBuffer;
	vkcv::DescriptorSetHandle m_shadowDescriptorSet;

	vkcv::PassHandle        m_shadowMapPass;
	vkcv::PipelineHandle    m_shadowMapPipe;
};
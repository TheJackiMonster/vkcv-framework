#pragma once
#include <vkcv/Core.hpp>
#include <vkcv/camera/Camera.hpp>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL // use this before inclusion, else error!
#include <glm/gtx/transform.hpp>

struct LightInfo {
	glm::vec3   direction;
	float       padding;
	glm::vec3   sunColor;
	float       sunStrength;
	glm::mat4   lightMatrix;
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

	vkcv::Image                         m_shadowMap;
	vkcv::Image                         m_shadowMapIntermediate;
	vkcv::Image                         m_shadowMapDepth;
	vkcv::SamplerHandle                 m_shadowSampler;
	vkcv::Buffer<LightInfo>             m_lightInfoBuffer;

	vkcv::PassHandle                    m_shadowMapPass;
	vkcv::PipelineHandle                m_shadowMapPipe;

	vkcv::PipelineHandle                m_depthToMomentsPipe;
	vkcv::DescriptorSetLayoutHandle     m_depthToMomentsDescriptorSetLayout;
	vkcv::DescriptorSetHandle           m_depthToMomentsDescriptorSet;

	vkcv::PipelineHandle                m_shadowBlurXPipe;
	vkcv::DescriptorSetLayoutHandle     m_shadowBlurXDescriptorSetLayout;
	vkcv::DescriptorSetHandle           m_shadowBlurXDescriptorSet;

	vkcv::PipelineHandle                m_shadowBlurYPipe;
	vkcv::DescriptorSetLayoutHandle     m_shadowBlurYDescriptorSetLayout;
	vkcv::DescriptorSetHandle           m_shadowBlurYDescriptorSet;
};
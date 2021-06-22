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
};

class ShadowMapping {
public:
	ShadowMapping(vkcv::Core* corePtr, const vkcv::VertexLayout& vertexLayout);

	void recordShadowMapRendering(
		const vkcv::CommandStreamHandle&    cmdStream,
		const glm::vec2&                    lightAngleRadian,
		const glm::vec3&                    lightColor,
		const float                         lightStrength,
		const std::vector<vkcv::Mesh>&      meshes,
		const std::vector<glm::mat4>&       modelMatrices,
		const vkcv::camera::Camera&         camera);

	vkcv::ImageHandle   getShadowMap();
	vkcv::SamplerHandle getShadowSampler();
	vkcv::BufferHandle  getLightInfoBuffer();

private:
	vkcv::Core* m_corePtr;

	vkcv::Image             m_shadowMap;
	vkcv::SamplerHandle     m_shadowSampler;
	vkcv::Buffer<LightInfo> m_lightInfoBuffer;

	vkcv::PassHandle        m_shadowMapPass;
	vkcv::PipelineHandle    m_shadowMapPipe;
};
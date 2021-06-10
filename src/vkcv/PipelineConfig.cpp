/**
 * @authors Mara Vogt, Mark Mints
 * @file src/vkcv/Pipeline.cpp
 * @brief Pipeline class to handle shader stages
 */

#include "vkcv/PipelineConfig.hpp"

namespace vkcv {

    PipelineConfig::PipelineConfig(
		const ShaderProgram&                        shaderProgram,
		uint32_t                                    width,
		uint32_t                                    height,
		const PassHandle                            &passHandle,
		const std::vector<VertexAttribute>          &vertexAttributes,
		const std::vector<vk::DescriptorSetLayout>  &descriptorLayouts,
		bool                                        useDynamicViewport,
		bool                                        useConservativeRasterization,
        PrimitiveTopology                           primitiveTopology)
		:
		m_ShaderProgram(shaderProgram),
		m_Height(height),
		m_Width(width),
		m_PassHandle(passHandle),
		m_VertexAttributes(vertexAttributes),
		m_DescriptorLayouts(descriptorLayouts),
		m_UseDynamicViewport(useDynamicViewport),
		m_UseConservativeRasterization(useConservativeRasterization),
        m_PrimitiveTopology(primitiveTopology)
		{}
}

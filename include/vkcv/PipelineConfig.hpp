#pragma once
/**
 * @authors Mara Vogt, Mark Mints
 * @file src/vkcv/Pipeline.hpp
 * @brief Pipeline class to handle shader stages
 */

#include <vector>
#include <cstdint>
#include "Handles.hpp"
#include "ShaderProgram.hpp"
#include "VertexLayout.hpp"

namespace vkcv {

    struct PipelineConfig {
        ShaderProgram                         m_ShaderProgram;
        uint32_t                              m_Width;
		uint32_t                              m_Height;
        PassHandle                            m_PassHandle;
        VertexLayout                          m_VertexLayout;
        std::vector<vk::DescriptorSetLayout>  m_DescriptorLayouts;
        bool                                  m_UseDynamicViewport;

    };

}
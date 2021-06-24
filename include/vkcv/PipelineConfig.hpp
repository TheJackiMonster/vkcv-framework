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
#include "ImageConfig.hpp"

namespace vkcv {

    enum class PrimitiveTopology{PointList, LineList, TriangleList };
	enum class CullMode{ None, Front, Back };
    enum class DepthTest { None, Less, LessEqual, Greater, GreatherEqual, Equal };

    struct PipelineConfig {
        ShaderProgram                           m_ShaderProgram;
        uint32_t                                m_Width;
		uint32_t                                m_Height;
        PassHandle                              m_PassHandle;
        VertexLayout                            m_VertexLayout;
        std::vector<vk::DescriptorSetLayout>    m_DescriptorLayouts;
        bool                                    m_UseDynamicViewport;
        bool                                    m_UseConservativeRasterization  = false;
        PrimitiveTopology                       m_PrimitiveTopology             = PrimitiveTopology::TriangleList;
        bool                                    m_EnableDepthClamping           = false;
        Multisampling                           m_multisampling                 = Multisampling::None;
        CullMode                                m_culling                       = CullMode::None;
        DepthTest                               m_depthTest                     = DepthTest::LessEqual;
        bool                                    m_depthWrite              = true;
    };

}
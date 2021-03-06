#pragma once
/**
 * @authors Mara Vogt, Mark Mints, Tobias Frisch
 * @file vkcv/GraphicsPipelineConfig.hpp
 * @brief Graphics Pipeline Config Struct to hand over required information to Pipeline Creation
 */

#include <vector>
#include <cstdint>

#include "Handles.hpp"
#include "ShaderProgram.hpp"
#include "VertexLayout.hpp"
#include "ImageConfig.hpp"

namespace vkcv {

	/**
	 * @brief Enum class to specify types of primitive topology.
	 */
    enum class PrimitiveTopology {
		PointList,
		LineList,
		TriangleList,
		PatchList
	};
	
	/**
	 * @brief Enum class to specify modes of culling.
	 */
	enum class CullMode {
		None,
		Front,
		Back,
		Both
	};
	
	/**
	 * @brief Enum class to specify depth-test modes.
	 */
    enum class DepthTest {
		None,
		Less,
		LessEqual,
		Greater,
		GreatherEqual,
		Equal
	};

    // add more as needed
    // alternatively we could expose the blend factors directly
	/**
	 * @brief Enum class to specify blending modes.
	 */
    enum class BlendMode {
		None,
		Additive
	};
	
	/**
	 * @brief Structure to configure a graphics pipeline before its creation.
	 */
    struct GraphicsPipelineConfig {
        ShaderProgram                         	m_ShaderProgram;
        uint32_t                              	m_Width;
		uint32_t                              	m_Height;
        PassHandle                            	m_PassHandle;
        VertexLayout                          	m_VertexLayout;
        std::vector<DescriptorSetLayoutHandle>	m_DescriptorLayouts;
        bool                                  	m_UseDynamicViewport;
        bool                                  	m_UseConservativeRasterization 	= false;
        PrimitiveTopology                     	m_PrimitiveTopology 			= PrimitiveTopology::TriangleList;
		BlendMode                             	m_blendMode 					= BlendMode::None;
        bool                                    m_EnableDepthClamping           = false;
        Multisampling                           m_multisampling                 = Multisampling::None;
        CullMode                                m_culling                       = CullMode::None;
        DepthTest                               m_depthTest                     = DepthTest::LessEqual;
        bool                                    m_depthWrite                    = true;
        bool                                    m_alphaToCoverage               = false;
		uint32_t								m_tessellationControlPoints		= 0;
    };

}
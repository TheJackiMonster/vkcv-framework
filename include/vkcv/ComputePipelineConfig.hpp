#pragma once
/**
 * @authors Mark Mints, Tobias Frisch
 * @file vkcv/ComputePipelineConfig.hpp
 * @brief Compute pipeline config struct to hand over required information to pipeline creation.
 */

#include <vector>

#include "Handles.hpp"
#include "ShaderProgram.hpp"

namespace vkcv {
	
	/**
	 * @brief Structure to configure a compute pipeline before its creation.
	 */
    struct ComputePipelineConfig {
        ShaderProgram&                          m_ShaderProgram;
        std::vector<DescriptorSetLayoutHandle>	m_DescriptorSetLayouts;
    };
	
}
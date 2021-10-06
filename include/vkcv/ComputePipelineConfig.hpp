#pragma once
/**
 * @authors Mark Mints, Tobias Frisch
 * @file vkcv/ComputePipelineConfig.hpp
 * @brief Compute pipeline config struct to hand over required information to pipeline creation.
 */

#include <vector>

#include "ShaderProgram.hpp"

namespace vkcv {
	
    struct ComputePipelineConfig {
        ShaderProgram&                          m_ShaderProgram;
        std::vector<vk::DescriptorSetLayout>  	m_DescriptorSetLayouts;
    };
	
}
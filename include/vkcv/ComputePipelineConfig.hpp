#pragma once
/**
 * @authors Mark Mints
 * @file src/vkcv/ComputePipelineConfig.hpp
 * @brief Compute Pipeline Config Struct to hand over required information to Pipeline Creation.
 */

#include <vector>
#include "ShaderProgram.hpp"

namespace vkcv
{
    struct ComputePipelineConfig {
        ShaderProgram&                          m_ShaderProgram;
        std::vector<vk::DescriptorSetLayout>  	m_DescriptorSetLayouts;
    };
}
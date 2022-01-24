#pragma once
/**
 * @authors Mark Mints, Tobias Frisch
 * @file src/vkcv/ComputePipelineConfig.hpp
 * @brief Compute Pipeline Config Struct to hand over required information to Pipeline Creation.
 */

#include <vector>

#include "Handles.hpp"
#include "ShaderProgram.hpp"

namespace vkcv
{
    struct ComputePipelineConfig {
        ShaderProgram&                          m_ShaderProgram;
        std::vector<DescriptorSetLayoutHandle>	m_DescriptorSetLayouts;
    };
}
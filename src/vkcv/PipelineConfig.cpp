/**
 * @authors Mara Vogt, Mark Mints
 * @file src/vkcv/Pipeline.cpp
 * @brief Pipeline class to handle shader stages
 */

#include "vkcv/PipelineConfig.hpp"

namespace vkcv {

    PipelineConfig::PipelineConfig(const ShaderProgram& shaderProgram, uint32_t width, uint32_t height, PassHandle &passHandle):
		m_shaderProgram(shaderProgram), m_height(height), m_width(width), m_passHandle(passHandle) {}
}

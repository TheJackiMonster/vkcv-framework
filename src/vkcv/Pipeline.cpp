/**
 * @authors Mara Vogt, Mark Mints
 * @file src/vkcv/Pipeline.cpp
 * @brief Pipeline class to handle shader stages
 */

#include "vkcv/Pipeline.hpp"

namespace vkcv {

    Pipeline::Pipeline(const ShaderProgram& shaderProgram, uint32_t width, uint32_t height, RenderpassHandle &passHandle):
		m_shaderProgram(shaderProgram), m_height(height), m_width(width), m_passHandle(passHandle) {}
}

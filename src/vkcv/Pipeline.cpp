/**
 * @authors Mara Vogt, Mark Mints
 * @file src/vkcv/Pipeline.cpp
 * @brief Pipeline class to handle shader stages
 */

#include "vkcv/Pipeline.hpp"

namespace vkcv {

    Pipeline::Pipeline(const std::vector<uint32_t> &vertexCode, const std::vector<uint32_t> &fragCode, uint32_t height, uint32_t width, PassHandle &passHandle):
        m_vertexCode(vertexCode), m_fragCode(fragCode), m_height(height), m_width(width), m_passHandle(passHandle) {}
}

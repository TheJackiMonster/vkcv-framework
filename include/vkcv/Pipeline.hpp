/**
 * @authors Mara Vogt, Mark Mints
 * @file src/vkcv/Pipeline.hpp
 * @brief Pipeline class to handle shader stages
 */

#ifndef VKCV_PIPELINE_HPP
#define VKCV_PIPELINE_HPP

#include <vector>
#include <cstdint>
#include "vkcv/Handles.hpp"

namespace vkcv {

    class Pipeline {

    public:
        /**
         *  Default constructer is deleted!
         */
        Pipeline() = delete;

        /**
         *  Constructor for the pipeline. Creates a pipeline using @p vertexCode, @p fragmentCode as well as the
         *  dimensions of the application window @p width and @p height. A handle for the Render Pass is also needed, @p passHandle.
         *
         * @param vertexCode Spir-V of Vertex Shader
         * @param fragCode Spir-V of Fragment Shader
         * @param height height of the application window
         * @param width width of the application window
         * @param passHandle handle for Render Pass
         */
        Pipeline(const std::vector<uint32_t> &vertexCode, const std::vector<uint32_t> &fragCode, uint32_t height, uint32_t width, PassHandle &passHandle);

        std::vector<uint32_t> m_vertexCode;
        std::vector<uint32_t> m_fragCode;
        uint32_t m_height;
        uint32_t m_width;
        PassHandle m_passHandle;
    };

}
#endif //VKCV_PIPELINE_HPP

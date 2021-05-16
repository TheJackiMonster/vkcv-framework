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
#include "ShaderProgram.hpp"

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
         * @param shaderProgram shaders of the pipeline
         * @param height height of the application window
         * @param width width of the application window
         * @param passHandle handle for Render Pass
         */
        Pipeline(const ShaderProgram& shaderProgram, uint32_t width, uint32_t height, RenderpassHandle &passHandle);

		ShaderProgram m_shaderProgram;
        uint32_t m_height;
        uint32_t m_width;
        RenderpassHandle m_passHandle;
    };

}
#endif //VKCV_PIPELINE_HPP

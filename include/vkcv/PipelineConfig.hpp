#pragma once
/**
 * @authors Mara Vogt, Mark Mints
 * @file src/vkcv/Pipeline.hpp
 * @brief Pipeline class to handle shader stages
 */

#include <vector>
#include <cstdint>
#include "vkcv/Handles.hpp"
#include "ShaderProgram.hpp"
#include <vkcv/VertexLayout.hpp>

namespace vkcv {

    struct PipelineConfig {
        /**
         *  Constructor for the pipeline. Creates a pipeline using @p vertexCode, @p fragmentCode as well as the
         *  dimensions of the application window @p width and @p height. A handle for the Render Pass is also needed, @p passHandle.
         *
         * @param shaderProgram shaders of the pipeline
         * @param height height of the application window
         * @param width width of the application window
         * @param passHandle handle for Render Pass
         */
        PipelineConfig(
            const ShaderProgram&                        shaderProgram,
            uint32_t                                    width,
            uint32_t                                    height,
            const PassHandle                            &passHandle,
            const std::vector<VertexAttribute>          &vertexAttributes,
            const std::vector<vk::DescriptorSetLayout>  &descriptorLayouts,
            bool                                        useDynamicViewport,
            bool                                        useConservativeRasterization = false);

        ShaderProgram                           m_ShaderProgram;
        uint32_t                                m_Height;
        uint32_t                                m_Width;
        PassHandle                              m_PassHandle;
        std::vector<VertexAttribute>            m_VertexAttributes;
        std::vector<vk::DescriptorSetLayout>    m_DescriptorLayouts;
        bool                                    m_UseDynamicViewport;
        bool                                    m_UseConservativeRasterization;
    };

}
#pragma once
/**
 * @authors Simeon Hermann, Leonie Franken
 * @file src/vkcv/ShaderProgram.hpp
 * @brief ShaderProgram class to handle and prepare the shader stages for a graphics pipeline
 */

#include <unordered_map>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <vulkan/vulkan.hpp>
#include <spirv_cross.hpp>
#include "VertexLayout.hpp"
#include "DescriptorConfig.hpp"

namespace vkcv {

    struct Shader
    {
        std::vector<char> shaderCode;
        vk::ShaderStageFlags shaderStage;
    };

	class ShaderProgram
	{
    public:
        ShaderProgram() noexcept; // ctor
        ~ShaderProgram() = default; // dtor

        /**
        * Adds a shader into the shader program.
        * The shader is only added if the shader program does not contain the particular shader stage already.
        * Contains: (1) reading of the code, (2) creation of a shader module, (3) creation of a shader stage, (4) adding to the shader stage list, (5) destroying of the shader module
        * @param[in] flag that signals the respective shaderStage (e.g. VK_SHADER_STAGE_VERTEX_BIT)
        * @param[in] relative path to the shader code (e.g. "../../../../../shaders/vert.spv")
        */
        bool addShader(vk::ShaderStageFlagBits shaderStage, const std::filesystem::path &shaderPath);

        /**
        * Returns the shader program's shader of the specified shader.
        * Needed for the transfer to the pipeline.
        * @return Shader object consisting of buffer with shader code and shader stage enum
        */
        const Shader &getShader(vk::ShaderStageFlagBits shaderStage) const;

        bool existsShader(vk::ShaderStageFlagBits shaderStage) const;

        const std::vector<VertexAttachment> &getVertexAttachments() const;
		size_t getPushConstantSize() const;

        const std::unordered_map<uint32_t, std::unordered_map<uint32_t, DescriptorBinding>>& getReflectedDescriptors() const;

	private:
	    /**
	     * Called after successfully adding a shader to the program.
	     * Fills vertex input attachments and descriptor sets (if present).
	     * @param shaderStage the stage to reflect data from
	     */
        void reflectShader(vk::ShaderStageFlagBits shaderStage);

        std::unordered_map<vk::ShaderStageFlagBits, Shader> m_Shaders;

        // contains all vertex input attachments used in the vertex buffer
        std::vector<VertexAttachment> m_VertexAttachments;

        /**
         * Map of map structure
         * First Key: Set ID, return an unordered map to the corresponding set's bindings
         * Second Key: Binding ID, returns the actual descriptor binding description
         * Value: descriptor binding description
         */
        std::unordered_map<uint32_t, std::unordered_map<uint32_t, DescriptorBinding>> m_DescriptorSets;

		size_t m_pushConstantSize = 0;
	};
}

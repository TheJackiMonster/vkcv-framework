#pragma once
/**
 * @authors Artur Wasmut, Leonie Franken, Tobias Frisch, Simeon Hermann, Alexander Gauggel, Mark Mints
 * @file vkcv/ShaderProgram.hpp
 * @brief ShaderProgram class to handle and prepare the shader stages for a graphics pipeline.
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
#include "ShaderStage.hpp"

namespace vkcv {

	class ShaderProgram
	{
    public:
        ShaderProgram() noexcept; // ctor
        ~ShaderProgram() = default; // dtor

        /**
         * @brief Adds a shader into the shader program.
         * The shader is only added if the shader program does not contain
         * the particular shader stage already.
         * Contains:
         * (1) reading the SPIR-V file,
         * (2) creating a shader module,
         * (3) creating a shader stage,
         * (4) adding to the shader stage list,
         * (5) destroying of the shader module
         *
         * @param[in] stage The stage of the shader
         * @param[in] path Path to the SPIR-V shader file
         */
        bool addShader(ShaderStage stage, const std::filesystem::path &path);

        /**
         * @brief Returns the shader binary of a specified stage from the program.
         * Needed for the transfer to the pipeline.
         *
         * @param[in] stage The stage of the shader
         * @return Shader code binary of the given stage
         */
        const std::vector<uint32_t> &getShaderBinary(ShaderStage stage) const;

		/**
		 * @brief Returns whether a shader exists in the program for a
		 * specified shader stage.
		 *
		 * @param[in] stage The stage of the shader
		 * @return True, if a shader exists for the stage, else false
		 */
        bool existsShader(ShaderStage stage) const;

		/**
		 * @brief Returns the vertex attachments for the program and its
		 * shader stages.
		 *
		 * @return Vertex attachments
		 */
        const std::vector<VertexAttachment> &getVertexAttachments() const;
		
		/**
		 * @brief Returns the size of the programs push constants.
		 *
		 * @return Size of push constants
		 */
		size_t getPushConstantsSize() const;

		/**
		 * @brief Returns the reflected descriptor set bindings mapped via
		 * their descriptor set id.
		 *
		 * @return Reflected descriptor set bindings
		 */
		const std::unordered_map<uint32_t, DescriptorBindings>& getReflectedDescriptors() const;

	private:
	    /**
	     * @brief Called after successfully adding a shader to the program.
	     * Fills vertex input attachments and descriptor sets (if present).
	     *
	     * @param[in] shaderStage the stage to reflect data from
	     */
        void reflectShader(ShaderStage shaderStage);

        std::unordered_map<ShaderStage, std::vector<uint32_t> > m_Shaders;

        // contains all vertex input attachments used in the vertex buffer
        std::vector<VertexAttachment> m_VertexAttachments;
        std::unordered_map<uint32_t, DescriptorBindings> m_DescriptorSets;
		size_t m_pushConstantsSize = 0;
	};

}

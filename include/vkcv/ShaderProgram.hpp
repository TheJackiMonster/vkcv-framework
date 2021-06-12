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
#include "vkcv/VertexLayout.hpp"
#include "vkcv/ShaderStage.hpp"
#include "vkcv/DescriptorConfig.hpp"

namespace vkcv {

    struct Shader
    {
        std::vector<char> shaderCode;
        ShaderStage shaderStage;
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
        bool addShader(ShaderStage shaderStage, const std::filesystem::path &shaderPath);

        /**
        * Returns the shader program's shader of the specified shader.
        * Needed for the transfer to the pipeline.
        * @return Shader object consisting of buffer with shader code and shader stage enum
        */
        const Shader &getShader(ShaderStage shaderStage) const;

        bool existsShader(ShaderStage shaderStage) const;

        void reflectShader(ShaderStage shaderStage);

        const VertexLayout &getVertexLayout() const;
		size_t getPushConstantSize() const;

        const std::vector<std::vector<DescriptorBinding>>& getReflectedDescriptors() const;

	private:
        std::unordered_map<ShaderStage, Shader> m_Shaders;

        VertexLayout m_VertexLayout;
        std::vector<std::vector<DescriptorBinding>> m_DescriptorSets;
		size_t m_pushConstantSize = 0;
	};
}

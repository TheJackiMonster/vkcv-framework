#pragma once
/**
 * @authors Simeon Hermann, Leonie Franken
 * @file src/vkcv/ShaderProgram.hpp
 * @brief ShaderProgram class to handle and prepare the shader stages for a graphics pipeline
 */

#define GLFW_INCLUDE_VULKAN
#include <vector>
#include <fstream>
#include <iostream>
#include <vulkan/vulkan.hpp>
#include "Context.hpp"

namespace vkcv {

	class ShaderProgram final {
	private:

		vkcv::Context& m_context;
		std::vector<vk::PipelineShaderStageCreateInfo> m_shaderStagesList;
		/**
		* Constructor of ShaderProgram requires a context for the logical device. 
		* @param context of the app
		*/
		ShaderProgram(vkcv::Context& context);
		
		/**
		* Reads the file of a given shader code. 
		* Only used within the class. 
		* @param[in] relative path to the shader code 
		* @return vector of chars as a buffer for the code 
		*/
		std::vector<char> readFile(const std::string& filepath);

		/**
		* Creates a shader module that encapsulates the read shader code. 
		* Only used within the class. 
		* Shader modules are destroyed after respective shader stages are created. 
		* @param[in] a vector of chars as a buffer for the code 
		* @return shader module 
		*/
		vk::ShaderModule createShaderModule(const std::vector<char>& shaderCode);

		/**
		* Creates a shader stage (info struct) for the to be added shader. 
		* Only used within the class. 
		* @param[in] Shader module that encapsulates the shader code 
		* @param[in] flag that signals the respective shaderStage 
		* @return pipeline shader stage info struct 
		*/ 
		vk::PipelineShaderStageCreateInfo createShaderStage(vk::ShaderModule& shaderModule, vk::ShaderStageFlagBits shaderStage);


	public:

		/**
		* destructor of ShaderProgram, does nothing so far
		*/
		~ShaderProgram();

		/**
		* Creates a shader program.
		* So far it only calls the constructor.
		* @param[in] context of the app
		*/
		static ShaderProgram create(vkcv::Context& context);

		/**
		* Adds a shader into the shader program.
		* The shader is only added if the shader program does not contain the particular shader stage already.
		* Contains: (1) reading of the code, (2) creation of a shader module, (3) creation of a shader stage, (4) adding to the shader stage list, (5) destroying of the shader module
		* @param[in] flag that signals the respective shaderStage (e.g. VK_SHADER_STAGE_VERTEX_BIT)
		* @param[in] relative path to the shader code (e.g. "../../../../../shaders/vert.spv")
		*/
		void addShader(vk::ShaderStageFlagBits shaderStage, const std::string& filepath);

		/**
		* Tests if the shader program contains a certain shader stage.
		* @param[in] flag that signals the respective shader stage (e.g. VK_SHADER_STAGE_VERTEX_BIT)
		* @return boolean that is true if the shader program contains the shader stage
		*/
		bool containsShaderStage(vk::ShaderStageFlagBits shaderStage);

		/**
		* Deletes the given shader stage in the shader program.
		* @param[in] flag that signals the respective shader stage (e.g. VK_SHADER_STAGE_VERTEX_BIT)
		* @return boolean that is false if the shader stage was not found in the shader program
		*/
		bool deleteShaderStage(vk::ShaderStageFlagBits shaderStage);

		/**
		* Returns a list with all the shader stages in the shader program.
		* Needed for the transfer to the pipeline.
		* @return vector list with all shader stage info structs
		*/
		std::vector<vk::PipelineShaderStageCreateInfo> getShaderStages();

		/**
		* Returns the number of shader stages in the shader program.
		* Needed for the transfer to the pipeline.
		* @return integer with the number of stages
		*/
		int getShaderStagesCount();


	};
}

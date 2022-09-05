#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/PipelineConfig.hpp
 * @brief Pipeline config class to hand over required information to pipeline creation
 */

#include <vector>

#include "Handles.hpp"
#include "ShaderProgram.hpp"

namespace vkcv {
	
	/**
	 * @brief Class to configure a general pipeline before its creation.
	 */
	class PipelineConfig {
	private:
		ShaderProgram m_ShaderProgram;
		std::vector<DescriptorSetLayoutHandle> m_DescriptorSetLayouts;
		
	public:
		PipelineConfig();
		
		PipelineConfig(const ShaderProgram& program,
					   const std::vector<DescriptorSetLayoutHandle>& layouts);
		
		PipelineConfig(const PipelineConfig &other) = default;
		PipelineConfig(PipelineConfig &&other) = default;
		
		~PipelineConfig() = default;
		
		PipelineConfig& operator=(const PipelineConfig &other) = default;
		PipelineConfig& operator=(PipelineConfig &&other) = default;
		
		void setShaderProgram(const ShaderProgram& program);
		
		[[nodiscard]]
		const ShaderProgram& getShaderProgram() const;
		
		void addDescriptorSetLayout(const DescriptorSetLayoutHandle& layout);
		
		void addDescriptorSetLayouts(const std::vector<DescriptorSetLayoutHandle>& layouts);
		
		[[nodiscard]]
		const std::vector<DescriptorSetLayoutHandle>& getDescriptorSetLayouts() const;
		
	};
	
}

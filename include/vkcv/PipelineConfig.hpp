#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/PipelineConfig.hpp
 * @brief Pipeline config class to hand over required information to pipeline creation
 */

#include "Container.hpp"
#include "Handles.hpp"
#include "ShaderProgram.hpp"

namespace vkcv {

	/**
	 * @brief Class to configure a general pipeline before its creation.
	 */
	class PipelineConfig {
	private:
		ShaderProgram m_ShaderProgram;
		Vector<DescriptorSetLayoutHandle> m_DescriptorSetLayouts;

	public:
		PipelineConfig();

		PipelineConfig(const ShaderProgram &program,
					   const Vector<DescriptorSetLayoutHandle> &layouts);

		PipelineConfig(const PipelineConfig &other) = default;
		PipelineConfig(PipelineConfig &&other) = default;

		~PipelineConfig() = default;

		PipelineConfig &operator=(const PipelineConfig &other) = default;
		PipelineConfig &operator=(PipelineConfig &&other) = default;

		void setShaderProgram(const ShaderProgram &program);

		[[nodiscard]] const ShaderProgram &getShaderProgram() const;

		void addDescriptorSetLayout(const DescriptorSetLayoutHandle &layout);

		void addDescriptorSetLayouts(const Vector<DescriptorSetLayoutHandle> &layouts);

		[[nodiscard]] const Vector<DescriptorSetLayoutHandle> &getDescriptorSetLayouts() const;
	};

} // namespace vkcv

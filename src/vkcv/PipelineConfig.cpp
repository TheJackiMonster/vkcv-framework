
#include "vkcv/PipelineConfig.hpp"

namespace vkcv {

	PipelineConfig::PipelineConfig() : m_ShaderProgram(), m_DescriptorSetLayouts() {}

	PipelineConfig::PipelineConfig(const ShaderProgram &program,
								   const std::vector<DescriptorSetLayoutHandle> &layouts) :
		m_ShaderProgram(program),
		m_DescriptorSetLayouts(layouts) {}

	void PipelineConfig::setShaderProgram(const ShaderProgram &program) {
		m_ShaderProgram = program;
	}

	const ShaderProgram &PipelineConfig::getShaderProgram() const {
		return m_ShaderProgram;
	}

	void PipelineConfig::addDescriptorSetLayout(const DescriptorSetLayoutHandle &layout) {
		m_DescriptorSetLayouts.push_back(layout);
	}

	void
	PipelineConfig::addDescriptorSetLayouts(const std::vector<DescriptorSetLayoutHandle> &layouts) {
		m_DescriptorSetLayouts.reserve(m_DescriptorSetLayouts.size() + layouts.size());

		for (const auto &layout : layouts) {
			m_DescriptorSetLayouts.push_back(layout);
		}
	}

	const std::vector<DescriptorSetLayoutHandle> &PipelineConfig::getDescriptorSetLayouts() const {
		return m_DescriptorSetLayouts;
	}

} // namespace vkcv


#include "vkcv/effects/GammaCorrectionEffect.hpp"

#include <vkcv/shader/GLSLCompiler.hpp>

#include "gammaCorrection.comp.hxx"

namespace vkcv::effects {
	
	static DescriptorBindings getDescriptorBindings() {
		DescriptorBindings descriptorBindings = {};
		
		auto binding_0 = DescriptorBinding {
				0,
				DescriptorType::IMAGE_STORAGE,
				1,
				ShaderStage::COMPUTE,
				false,
				false
		};
		
		auto binding_1 = DescriptorBinding {
				1,
				DescriptorType::IMAGE_STORAGE,
				1,
				ShaderStage::COMPUTE,
				false,
				false
		};
		
		descriptorBindings.insert(std::make_pair(0, binding_0));
		descriptorBindings.insert(std::make_pair(1, binding_1));
		
		return descriptorBindings;
	}
	
	GammaCorrectionEffect::GammaCorrectionEffect(Core &core)
	: Effect(core), m_gamma(2.2f), m_descriptorSetLayout(), m_descriptorSet(), m_pipeline() {
		vkcv::shader::GLSLCompiler compiler;
		ShaderProgram program;
		
		compiler.compileSource(
			ShaderStage::COMPUTE,
			GAMMACORRECTION_COMP_SHADER,
			[&program](ShaderStage stage, const std::filesystem::path &path) {
				program.addShader(stage, path);
			}
		);
		
		m_descriptorSetLayout = m_core.createDescriptorSetLayout(getDescriptorBindings());
		m_descriptorSet = m_core.createDescriptorSet(m_descriptorSetLayout);
		m_pipeline = m_core.createComputePipeline({
			program,
			{ m_descriptorSetLayout }
		});
	}
	
	void GammaCorrectionEffect::recordEffect(const CommandStreamHandle &cmdStream,
											 const ImageHandle &input,
											 const ImageHandle &output) {
		m_core.recordBeginDebugLabel(cmdStream, "Gamma Correction", std::array<float, 4>{
				0.95f, 0.95f, 0.95f, 1.0f
		});
		
		m_core.prepareImageForStorage(cmdStream, input);
		m_core.prepareImageForStorage(cmdStream, output);
		
		vkcv::DescriptorWrites writes;
		
		writes.writeStorageImage(0, input);
		writes.writeStorageImage(1, output);
		
		m_core.writeDescriptorSet(m_descriptorSet, writes);
		
		const uint32_t width = m_core.getImageWidth(output);
		const uint32_t height = m_core.getImageHeight(output);
		
		m_core.recordComputeDispatchToCmdStream(
				cmdStream,
				m_pipeline,
				dispatchInvocations(
						DispatchSize(width, height),
						DispatchSize(8, 8)
				),
				{ useDescriptorSet(0, m_descriptorSet) },
				pushConstants<float>(m_gamma)
		);
		
		m_core.recordEndDebugLabel(cmdStream);
	}
	
	void GammaCorrectionEffect::setGamma(float gamma) {
		m_gamma = std::max(gamma, std::numeric_limits<float>::epsilon());
	}
	
	float GammaCorrectionEffect::getGamma() const {
		return m_gamma;
	}
	
}

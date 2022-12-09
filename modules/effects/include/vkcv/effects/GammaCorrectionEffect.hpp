#pragma once

#include "Effect.hpp"

namespace vkcv::effects {
	
	class GammaCorrectionEffect : public Effect {
	private:
		float m_gamma;
		
		ComputePipelineHandle m_pipeline;
		
		DescriptorSetLayoutHandle m_descriptorSetLayout;
		
		DescriptorSetHandle m_descriptorSet;
	
	public:
		GammaCorrectionEffect(Core& core);
		
		void recordEffect(const CommandStreamHandle& cmdStream,
						  const ImageHandle& input,
						  const ImageHandle& output) override;
		
		void setGamma(float gamma);
		
		float getGamma() const;
		
	};
	
}

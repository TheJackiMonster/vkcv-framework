#pragma once

#include "Effect.hpp"

namespace vkcv::effects {
	
	/**
     * @addtogroup vkcv_effects
     * @{
     */
	
	class GammaCorrectionEffect : public Effect {
	private:
		float m_gamma;
		
		ComputePipelineHandle m_pipeline;
		
		DescriptorSetLayoutHandle m_descriptorSetLayout;
		
		DescriptorSetHandle m_descriptorSet;
	
	public:
		explicit GammaCorrectionEffect(Core& core);
		
		void recordEffect(const CommandStreamHandle& cmdStream,
						  const ImageHandle& input,
						  const ImageHandle& output) override;
		
		void setGamma(float gamma);
		
		[[nodiscard]]
		float getGamma() const;
		
	};
	
	/** @} */
	
}

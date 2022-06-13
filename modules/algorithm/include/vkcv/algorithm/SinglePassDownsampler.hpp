#pragma once

#include <vector>

#include <vkcv/Core.hpp>
#include <vkcv/Downsampler.hpp>
#include <vkcv/ShaderProgram.hpp>

namespace vkcv::algorithm {
	
	/**
	* @defgroup vkcv_algorithm Algorithm Module
	* A module to use different optimized algorithms.
	* @{
	*/

	/**
	 * @brief A class to handle downsampling via FidelityFX Single Pass Downsampler.
	 * https://github.com/GPUOpen-Effects/FidelityFX-SPD
	 */
	class SinglePassDownsampler : public vkcv::Downsampler {
	private:
		/**
		 * The SPD compute pipeline of the downsampler.
		 */
		ComputePipelineHandle m_pipeline;
		
		/**
         * The descriptor set layout of the SPD pipeline.
         */
		DescriptorSetLayoutHandle m_descriptorSetLayout;
		
		/**
		 * The vector of descriptor sets currently in use for downsampling.
		 */
		std::vector<DescriptorSetHandle> m_descriptorSets;
		
		/**
		 * The buffer template to handle global atomic counters for SPD.
		 */
		Buffer<uint32_t> m_globalCounter;
		
		/**
		 * The optional sampler handle to use for the downsampling.
		 */
		SamplerHandle m_sampler;
		
	public:
		/**
		 * @brief Constructor to create instance for single pass downsampling.
		 *
		 * @param[in,out] core Reference to a Core instance
		 * @param[in] sampler Sampler handle
		 */
		explicit SinglePassDownsampler(Core& core,
									   const SamplerHandle &sampler = SamplerHandle());
		
		/**
		 * @brief Record the commands of the downsampling instance to
		 * generate all mip levels of an input image via a
		 * command stream.
		 *
		 * @param[in] cmdStream Command stream handle
		 * @param[in] image Image handle
		 */
		void recordDownsampling(const CommandStreamHandle& cmdStream,
								const ImageHandle& image) override;
	
	};
	
	/** @} */

}

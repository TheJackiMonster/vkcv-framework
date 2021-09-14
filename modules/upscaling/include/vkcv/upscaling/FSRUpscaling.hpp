#pragma once

#include "Upscaling.hpp"

#include <vkcv/ShaderProgram.hpp>

namespace vkcv::upscaling {

    /**
     * @addtogroup vkcv_upscaling
     * @{
     */
	
	enum class FSRQualityMode : int {
		NONE = 0,
		ULTRA_QUALITY = 1,
		QUALITY = 2,
		BALANCED = 3,
		PERFORMANCE = 4
	};
	
	void getFSRResolution(FSRQualityMode mode,
						  uint32_t outputWidth, uint32_t outputHeight,
						  uint32_t &inputWidth, uint32_t &inputHeight);
	
	float getFSRLodBias(FSRQualityMode mode);
	
	struct FSRConstants {
		uint32_t Const0 [4];
		uint32_t Const1 [4];
		uint32_t Const2 [4];
		uint32_t Const3 [4];
		uint32_t Sample [4];
	};
	
	class FSRUpscaling : public Upscaling {
	private:
        /**
         * The EASU compute pipeline of the FSR upscaling.
         */
		ComputePipelineHandle m_easuPipeline;

        /**
         * The RCAS compute pipeline of the FSR upscaling.
         */
		ComputePipelineHandle m_rcasPipeline;

        /**
         * The descriptor set layout of the EASU pipeline.
         */
		DescriptorSetLayoutHandle m_easuDescriptorSetLayout;

        /**
         * The descriptor set for the EASU pipeline.
         */
		DescriptorSetHandle m_easuDescriptorSet;

        /**
         * The descriptor set layout of the RCAS pipeline.
         */
		DescriptorSetLayoutHandle m_rcasDescriptorSetLayout;

        /**
         * The descriptor set for the RCAS pipeline.
         */
		DescriptorSetHandle m_rcasDescriptorSet;

        /**
         * The buffer template to handle FSR constants for
         * the EASU pipeline.
         */
		Buffer<FSRConstants> m_easuConstants;

        /**
         * The buffer template to handle FSR constants for
         * the RCAS pipeline.
         */
		Buffer<FSRConstants> m_rcasConstants;

        /**
         * The image handle to store the intermidiate state of
         * the FSR upscaling.
         */
		ImageHandle m_intermediateImage;

        /**
         * The sampler handle to use for accessing the images
         * in the FSR upscaling process.
         */
		SamplerHandle m_sampler;

        /**
         * Current state of HDR support.
         */
		bool m_hdr;
		
		/**
		 * Sharpness will calculate the rcasAttenuation value
		 * which should be between 0.0f and 2.0f (default: 0.25f).
		 *
		 * rcasAttenuation = (1.0f - sharpness) * 2.0f
		 *
		 * So the default value for sharpness should be 0.875f.
		 *
		 * Beware that 0.0f or any negative value of sharpness will
		 * disable the rcas pass completely.
		 */
		float m_sharpness;
	
	public:
        /**
         * Constructor to create instance for FSR upscaling.
         * @param[in,out] core Reference to a Core instance
         */
		explicit FSRUpscaling(Core& core);

        /**
         * Record the comands of the FSR upscaling instance to
         * scale the image of the input handle to the resolution of
         * the output image handle via FidelityFX Super Resolution.
         * @param[in] cmdStream Command stream handle to record commands
         * @param[in] input Input image handle
         * @param[in] output Output image handle
         */
		void recordUpscaling(const CommandStreamHandle& cmdStream,
							 const ImageHandle& input,
							 const ImageHandle& output) override;

        /**
         * Checks if HDR support is enabled and returns the status as boolean.
         * @return true if HDR is supported, otherwise false
         */
		[[nodiscard]]
		bool isHdrEnabled() const;

        /**
         * Changes the status of HDR support of the FSR upscaling instance.
         * @param[in] enabled New status of HDR support
         */
		void setHdrEnabled(bool enabled);

        /**
         * Returns the amount of sharpness the FSR upscaling instance is using.
         * @return The amount of sharpness
         */
		[[nodiscard]]
		float getSharpness() const;

        /**
         * Changes the amount of sharpness of the FSR upscaling instance.
         * The new sharpness value is restricted by 0.0f as lower and 1.0f
         * as upper boundary.
         * @param[in] sharpness New sharpness value
         */
		void setSharpness(float sharpness);
		
	};

    /** @} */

}

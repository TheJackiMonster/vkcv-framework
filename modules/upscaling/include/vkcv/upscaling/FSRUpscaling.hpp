#pragma once

#include "Upscaling.hpp"

#include <vkcv/Buffer.hpp>
#include <vkcv/ShaderProgram.hpp>

namespace vkcv::upscaling {

    /**
     * @addtogroup vkcv_upscaling
     * @{
     */

    /**
     * Enum to set the mode of quality for
     * FSR upscaling.
     */
	enum class FSRQualityMode : int {
        /**
         * Don't upscale anything.
         */
		NONE = 0,

        /**
         * Highest quality of FSR upscaling:
         * 1.3x per dimension
         */
		ULTRA_QUALITY = 1,

        /**
         * High quality of FSR upscaling:
         * 1.5x per dimension
         */
		QUALITY = 2,

        /**
         * Medium quality of FSR upscaling:
         * 1.7x per dimension
         */
		BALANCED = 3,

        /**
         * Low quality of FSR upscaling:
         * 2.0x per dimension
         */
		PERFORMANCE = 4,
		
		/**
         * Lowest quality of FSR upscaling:
         * 3.0x per dimension
         */
		ULTRA_PERFORMANCE = 5,
	};

    /**
     * Calculates the internal resolution for actual rendering if
     * a specific mode of quality is used for upscaling with FSR.
     *
     * @param[in] mode Mode of quality
     * @param[in] outputWidth Final resolution width
     * @param[in] outputHeight Final resolution height
     * @param[out] inputWidth Internal resolution width
     * @param[out] inputHeight Internal resolution height
     */
	void getFSRResolution(FSRQualityMode mode,
						  uint32_t outputWidth, uint32_t outputHeight,
						  uint32_t &inputWidth, uint32_t &inputHeight);

    /**
     * Returns the matching negative lod bias to reduce artifacts
     * upscaling with FSR under a given mode of quality.
     *
     * @param mode Mode of quality
     * @return Lod bias
     */
	float getFSRLodBias(FSRQualityMode mode);

    /**
     * A structure to exchange required configuration
     * with the shaders used by FSR upscaling.
     */
	struct FSRConstants {
        /**
         * 0th FSR constant.
         */
		uint32_t Const0 [4];

        /**
         * 1st FSR constant.
         */
        uint32_t Const1 [4];

        /**
         * 2nd FSR constant.
         */
        uint32_t Const2 [4];

        /**
         * 3rd FSR constant.
         */
        uint32_t Const3 [4];

        /**
         * 4th FSR constant.
         */
        uint32_t Sample [4];
	};

    /**
     * A class to handle upscaling via FidelityFX Super Resolution.
     * https://github.com/GPUOpen-Effects/FidelityFX-FSR
     */
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
         *
         * @param[in,out] core Reference to a Core instance
         */
		explicit FSRUpscaling(Core& core);

        /**
         * Record the comands of the FSR upscaling instance to
         * scale the image of the input handle to the resolution of
         * the output image handle via FidelityFX Super Resolution.
         *
         * @param[in] cmdStream Command stream handle to record commands
         * @param[in] input Input image handle
         * @param[in] output Output image handle
         */
		void recordUpscaling(const CommandStreamHandle& cmdStream,
							 const ImageHandle& input,
							 const ImageHandle& output) override;

        /**
         * Checks if HDR support is enabled and returns the status as boolean.
         *
         * @return true if HDR is supported, otherwise false
         */
		[[nodiscard]]
		bool isHdrEnabled() const;

        /**
         * Changes the status of HDR support of the FSR upscaling instance.
         *
         * @param[in] enabled New status of HDR support
         */
		void setHdrEnabled(bool enabled);

        /**
         * Returns the amount of sharpness the FSR upscaling instance is using.
         *
         * @return The amount of sharpness
         */
		[[nodiscard]]
		float getSharpness() const;

        /**
         * Changes the amount of sharpness of the FSR upscaling instance.
         * The new sharpness value is restricted by 0.0f as lower and 1.0f
         * as upper boundary.
         *
         * @param[in] sharpness New sharpness value
         */
		void setSharpness(float sharpness);
		
	};

    /** @} */

}

#pragma once

#include "Upscaling.hpp"

#include <vector>

#define FFX_GCC
#include <ffx_fsr2.h>
#undef FFX_GCC

namespace vkcv::upscaling {

/**
     * @addtogroup vkcv_upscaling
     * @{
     */

	/**
     * A class to handle upscaling via FidelityFX Super Resolution.
     * https://github.com/GPUOpen-Effects/FidelityFX-FSR2
     */
	class FSR2Upscaling : public Upscaling {
	private:
		std::vector<char> m_scratchBuffer;
		
		FfxFsr2ContextDescription m_description;
		FfxFsr2Context m_context;
		
		ImageHandle m_depth;
		ImageHandle m_velocity;
		
		float m_frameDeltaTime;
		bool m_reset;
		
		float m_near;
		float m_far;
		float m_fov;
		
		/**
		* Current state of HDR support.
		*/
		bool m_hdr;
		
		/**
		 * Sharpness will improve the upscaled image quality with
		 * a factor between 0.0f for no sharpening and 1.0f for
		 * maximum sharpening.
		 *
		 * The default value for sharpness should be 0.875f.
		 *
		 * Beware that 0.0f or any negative value of sharpness will
		 * disable the sharpening pass completely.
		 */
		float m_sharpness;
		
		void createFSR2Context(uint32_t displayWidth,
							   uint32_t displayHeight,
							   uint32_t renderWidth,
							   uint32_t renderHeight);
		
		void destroyFSR2Context();
		
	public:
		/**
         * Constructor to create an instance for FSR upscaling.
         *
         * @param[in,out] core Reference to a Core instance
         */
		explicit FSR2Upscaling(Core& core);
		
		/**
		 * Destructor to free the instance for FSR upscaling.
		 */
		~FSR2Upscaling();
		
		/**
		 * Update the upscaling instance with current frame
		 * delta time and whether the temporal data needs to
		 * be reset (for example because the camera switched).
		 *
		 * @param[in] deltaTime Current frame delta time
		 * @param[in] reset Reset temporal frame data
		 */
		void update(float deltaTime, bool reset = false);
		
		/**
		 * Bind the depth buffer image to use with the FSR2
		 * upscaling instance for utilizing depth information.
		 *
		 * @param[in] depthInput Depth input image handle
		 */
		void bindDepthBuffer(const ImageHandle& depthInput);
		
		/**
		 * Bind the velocity buffer image to use with the FSR2
		 * upscaling instance for utilizing 2D motion vectors.
		 *
		 * @param[in] velocityInput Velocity input image handle
		 */
		void bindVelocityBuffer(const ImageHandle& velocityInput);
		
		/**
		 * Record the comands of the FSR2 upscaling instance to
		 * scale the image of the input handle to the resolution of
		 * the output image handle via FidelityFX Super Resolution.
		 *
		 * @param[in] cmdStream Command stream handle to record commands
		 * @param[in] colorInput Color input image handle
		 * @param[in] output Output image handle
		 */
		void recordUpscaling(const CommandStreamHandle& cmdStream,
							 const ImageHandle& colorInput,
							 const ImageHandle& output) override;
		
		/**
		 * Set the required camera values for the FSR2 upscaling
		 * instance including near- and far-plane as well as
		 * the FOV angle vertical.
		 *
		 * @param[in] near Camera near plane
		 * @param[in] far Camera far plane
		 * @param[in] fov Camera field of view angle vertical
		 */
		void setCamera(float near, float far, float fov);
		
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
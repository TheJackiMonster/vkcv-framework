#pragma once

#include <vkcv/Core.hpp>
#include <vkcv/Handles.hpp>

namespace vkcv::upscaling {

    /**
     * @defgroup vkcv_upscaling Upscaling Module
     * A module to upscale an image from an internal resolution to a final resolution in realtime.
     * @{
     */

    /**
     * An abstract class to handle upscaling of images in realtime.
     */
	class Upscaling {
	protected:
        /**
         * Reference to the current Core instance.
         */
		Core& m_core;
	
	public:
        /**
         * Constructor to create an upscaling instance.
         *
         * @param[in,out] core Reference to a Core instance
         */
		explicit Upscaling(Core& core);
		
		~Upscaling() = default;

        /**
         * Record the comands of the given upscaling instance to
         * scale the image of the input handle to the resolution of
         * the output image handle.
         *
         * @param[in] cmdStream Command stream handle to record commands
         * @param[in] input Input image handle
         * @param[in] output Output image handle
         */
		virtual void recordUpscaling(const CommandStreamHandle& cmdStream,
									 const ImageHandle& input,
							 		 const ImageHandle& output) = 0;
	
	};

    /** @} */
	
}

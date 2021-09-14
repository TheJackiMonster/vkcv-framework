#pragma once

#include "Upscaling.hpp"

namespace vkcv::upscaling {

    /**
     * @addtogroup vkcv_upscaling
     * @{
     */

    /**
     * A class to handle upscaling via bilinear interpolation.
     */
	class BilinearUpscaling : public Upscaling {
	private:
	public:
        /**
         * Constructor to create instance for bilinear upscaling.
         * @param[in,out] core Reference to a Core instance
         */
		explicit BilinearUpscaling(Core& core);

        /**
         * Record the comands of the bilinear upscaling instance to
         * scale the image of the input handle to the resolution of
         * the output image handle via bilinear interpolation.
         * @param[in] cmdStream Command stream handle to record commands
         * @param[in] input Input image handle
         * @param[in] output Output image handle
         */
		void recordUpscaling(const CommandStreamHandle& cmdStream,
							 const ImageHandle& input,
							 const ImageHandle& output) override;
	
	};

    /** @} */

}

#pragma once

#include <vkcv/Core.hpp>
#include <vkcv/Handles.hpp>

namespace vkcv::upscaling {

    /**
     * @defgroup vkcv_upscaling Upscaling Module
     * A module to upscale an image from an internal resolution to a final resolution in realtime.
     * @{
     */
	
	class Upscaling {
	protected:
		Core& m_core;
	
	public:
		Upscaling(Core& core);
		
		~Upscaling() = default;
		
		virtual void recordUpscaling(const CommandStreamHandle& cmdStream,
									 const ImageHandle& input,
							 		 const ImageHandle& output) = 0;
	
	};

    /** @} */
	
}

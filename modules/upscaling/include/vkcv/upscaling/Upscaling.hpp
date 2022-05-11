#pragma once

#include <vkcv/Core.hpp>
#include <vkcv/Handles.hpp>

namespace vkcv::upscaling {
	
	class Upscaling {
	protected:
		Core& m_core;
	
	public:
		explicit Upscaling(Core& core);
		
		~Upscaling() = default;
		
		virtual void recordUpscaling(const CommandStreamHandle& cmdStream,
									 const ImageHandle& input,
							 		 const ImageHandle& output) = 0;
	
	};
	
}

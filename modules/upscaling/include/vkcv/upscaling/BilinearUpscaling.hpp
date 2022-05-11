#pragma once

#include "Upscaling.hpp"

namespace vkcv::upscaling {
	
	class BilinearUpscaling : public Upscaling {
	private:
	public:
		explicit BilinearUpscaling(Core& core);
		
		void recordUpscaling(const CommandStreamHandle& cmdStream,
							 const ImageHandle& input,
							 const ImageHandle& output) override;
	
	};

}

#pragma once

#include "ToneMapping.hpp"

namespace vkcv::tone {
	
	/**
     * @addtogroup vkcv_tone
     * @{
     */
	
	class ReinhardToneMapping : public ToneMapping {
	private:
		void initToneMapping() override;
	
	public:
		explicit ReinhardToneMapping(Core& core, bool normalize = false);
		
	};
	
	/** @} */
	
}

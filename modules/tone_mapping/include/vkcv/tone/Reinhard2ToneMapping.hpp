#pragma once

#include "ToneMapping.hpp"

namespace vkcv::tone {
	
	/**
     * @addtogroup vkcv_tone
     * @{
     */
	
	class Reinhard2ToneMapping : public ToneMapping {
	private:
		void initToneMapping() override;
	
	public:
		explicit Reinhard2ToneMapping(Core& core, bool normalize = false);
		
	};
	
	/** @} */
	
}

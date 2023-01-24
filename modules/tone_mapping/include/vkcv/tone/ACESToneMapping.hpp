#pragma once

#include "ToneMapping.hpp"

namespace vkcv::tone {
	
	/**
     * @addtogroup vkcv_tone
     * @{
     */
	
	class ACESToneMapping : public ToneMapping {
	private:
		void initToneMapping() override;
		
	public:
		explicit ACESToneMapping(Core& core, bool normalize = false);
		
	};
	
	/** @} */
	
}

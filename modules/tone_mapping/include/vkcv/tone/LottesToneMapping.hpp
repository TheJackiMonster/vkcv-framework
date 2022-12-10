#pragma once

#include "ToneMapping.hpp"

namespace vkcv::tone {
	
	/**
     * @addtogroup vkcv_tone
     * @{
     */
	
	class LottesToneMapping : public ToneMapping {
	private:
		void initToneMapping() override;
	
	public:
		explicit LottesToneMapping(Core& core, bool normalize = false);
		
	};
	
	/** @} */
	
}

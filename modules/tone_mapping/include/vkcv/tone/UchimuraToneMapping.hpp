#pragma once

#include "ToneMapping.hpp"

namespace vkcv::tone {
	
	/**
     * @addtogroup vkcv_tone
     * @{
     */
	
	class UchimuraToneMapping : public ToneMapping {
	private:
		void initToneMapping() override;
	
	public:
		explicit UchimuraToneMapping(Core& core, bool normalize = false);
		
	};
	
	/** @} */
	
}

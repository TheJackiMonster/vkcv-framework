#pragma once

#include "ToneMapping.hpp"

namespace vkcv::tone {
	
	/**
     * @addtogroup vkcv_tone
     * @{
     */
	
	class FilmicToneMapping : public ToneMapping {
	private:
		void initToneMapping() override;
	
	public:
		explicit FilmicToneMapping(Core& core, bool normalize = false);
		
	};
	
	/** @} */
	
}

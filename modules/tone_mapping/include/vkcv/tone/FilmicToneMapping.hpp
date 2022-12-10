#pragma once

#include "ToneMapping.hpp"

namespace vkcv::tone {
	
	class FilmicToneMapping : public ToneMapping {
	private:
		void initToneMapping() override;
	
	public:
		explicit FilmicToneMapping(Core& core, bool normalize = false);
		
	};
	
}

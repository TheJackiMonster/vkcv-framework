#pragma once

#include "ToneMapping.hpp"

namespace vkcv::tone {
	
	class ACESToneMapping : public ToneMapping {
	private:
		void initToneMapping() override;
		
	public:
		explicit ACESToneMapping(Core& core);
		
	};
	
}

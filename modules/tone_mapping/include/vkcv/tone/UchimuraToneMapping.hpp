#pragma once

#include "ToneMapping.hpp"

namespace vkcv::tone {
	
	class UchimuraToneMapping : public ToneMapping {
	private:
		void initToneMapping() override;
	
	public:
		explicit UchimuraToneMapping(Core& core);
		
	};
	
}

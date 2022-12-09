#pragma once

#include "ToneMapping.hpp"

namespace vkcv::tone {
	
	class UnrealToneMapping : public ToneMapping {
	private:
		void initToneMapping() override;
	
	public:
		explicit UnrealToneMapping(Core& core);
		
	};
	
}

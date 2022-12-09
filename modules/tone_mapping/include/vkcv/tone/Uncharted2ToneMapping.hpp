#pragma once

#include "ToneMapping.hpp"

namespace vkcv::tone {
	
	class Uncharted2ToneMapping : public ToneMapping {
	private:
		void initToneMapping() override;
	
	public:
		explicit Uncharted2ToneMapping(Core& core);
		
	};
	
}

#pragma once

#include "ToneMapping.hpp"

namespace vkcv::tone {
	
	class LottesToneMapping : public ToneMapping {
	private:
		void initToneMapping() override;
	
	public:
		explicit LottesToneMapping(Core& core, bool normalize = false);
		
	};
	
}

#pragma once
#include <vkcv/Handles.hpp>

namespace vkcv::material {

	class Material {
	private:
	public:
		const DescriptorSetHandle m_DescriptorSetHandle;
	protected:
		Material(const DescriptorSetHandle& setHandle); 
	};
	
}

#include "RTX.hpp"

namespace vkcv::rtx {
	
	RTXModule::RTXModule(Core* core,
						 ASManager* asManager,
						 std::vector<vkcv::DescriptorSetHandle>& descriptorSetHandles){
		m_core = core;
		m_asManager = asManager;
		// build acceleration structures BLAS then TLAS --> see ASManager
		m_asManager->buildTLAS();
		RTXDescriptors(descriptorSetHandles);
	}
	
	
	RTXModule::~RTXModule()
	{}
	
	
	void RTXModule::RTXDescriptors(std::vector<vkcv::DescriptorSetHandle>& descriptorSetHandles)
	{
		//TLAS-Descriptor-Write
		{
			vkcv::DescriptorWrites writes;
			writes.writeAcceleration(1, { m_asManager->getTLAS().vulkanHandle });
			m_core->writeDescriptorSet(descriptorSetHandles[0], writes);
		}
	}

}
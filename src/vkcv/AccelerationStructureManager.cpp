
#include "AccelerationStructureManager.hpp"

#include "vkcv/Core.hpp"
#include "vkcv/Logger.hpp"

namespace vkcv {
	
	bool AccelerationStructureManager::init(Core &core, BufferManager &bufferManager) {
		if (!HandleManager<AccelerationStructureEntry, AccelerationStructureHandle>::init(core)) {
			return false;
		}
		
		m_bufferManager = &bufferManager;
		return true;
	}
	
	uint64_t AccelerationStructureManager::getIdFrom(const AccelerationStructureHandle &handle) const {
		return handle.getId();
	}
	
	AccelerationStructureHandle
	AccelerationStructureManager::createById(uint64_t id, const HandleDestroyFunction &destroy) {
		return AccelerationStructureHandle(id, destroy);
	}
	
	void AccelerationStructureManager::destroyById(uint64_t id) {
		auto &accelerationStructure = getById(id);
		
		if (accelerationStructure.m_accelerationStructure) {
			getCore().getContext().getDevice().destroyAccelerationStructureKHR(
					accelerationStructure.m_accelerationStructure,
					nullptr,
					getCore().getContext().getDispatchLoaderDynamic()
			);
			
			accelerationStructure.m_accelerationStructure = nullptr;
		}
	}
	
	const BufferManager &AccelerationStructureManager::getBufferManager() const {
		return *m_bufferManager;
	}
	
	BufferManager &AccelerationStructureManager::getBufferManager() {
		return *m_bufferManager;
	}
	
	AccelerationStructureManager::AccelerationStructureManager() noexcept :
		HandleManager<AccelerationStructureEntry, AccelerationStructureHandle>(),
		m_bufferManager(nullptr) {}
	
	AccelerationStructureManager::~AccelerationStructureManager() noexcept {
		clear();
	}
	
}

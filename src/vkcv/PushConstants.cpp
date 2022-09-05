#include <vkcv/PushConstants.hpp>

namespace vkcv {
	
	PushConstants::PushConstants(size_t sizePerDrawcall) :
	m_typeGuard(sizePerDrawcall),
	m_data()
	{}
	
	PushConstants::PushConstants(const TypeGuard &guard) :
	m_typeGuard(guard),
	m_data()
	{}
	
	size_t PushConstants::getSizePerDrawcall() const {
		return m_typeGuard.typeSize();
	}
	
	size_t PushConstants::getFullSize() const {
		return m_data.size();
	}
	
	size_t PushConstants::getDrawcallCount() const {
		return getFullSize() / getSizePerDrawcall();
	}
	
	void PushConstants::clear() {
		m_data.clear();
	}

	const void* PushConstants::getDrawcallData(size_t index) const {
		const size_t offset = (index * getSizePerDrawcall());
		return reinterpret_cast<const void*>(m_data.data() + offset);
	}

	const void* PushConstants::getData() const {
		if (m_data.empty()) {
			return nullptr;
		} else {
			return m_data.data();
		}
	}

}

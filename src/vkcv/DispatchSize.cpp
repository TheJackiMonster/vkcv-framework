
#include "vkcv/DispatchSize.hpp"

#include <cmath>

namespace vkcv {
	
	DispatchSize::DispatchSize(uint32_t count)
	: DispatchSize(count, 1, 1)
	{}
	
	DispatchSize::DispatchSize(uint32_t dimensionX, uint32_t dimentionY, uint32_t dimensionZ)
	: m_Dispatch({ dimensionX, dimentionY, dimensionZ })
	{}
	
	const uint32_t *DispatchSize::data() const {
		return m_Dispatch.data();
	}
	
	uint32_t DispatchSize::operator[](size_t index) const {
		return m_Dispatch.at(index);
	}
	
	uint32_t DispatchSize::x() const {
		return m_Dispatch[0];
	}
	
	uint32_t DispatchSize::y() const {
		return m_Dispatch[1];
	}
	
	uint32_t DispatchSize::z() const {
		return m_Dispatch[2];
	}
	
	DispatchSize dispatchInvocations(DispatchSize globalInvocations, DispatchSize groupSize) {
		const uint32_t dimensionX = std::ceil(1.0f * globalInvocations.x() / groupSize.x());
		const uint32_t dimensionY = std::ceil(1.0f * globalInvocations.y() / groupSize.y());
		const uint32_t dimensionZ = std::ceil(1.0f * globalInvocations.z() / groupSize.z());
		
		return DispatchSize(dimensionX, dimensionY, dimensionZ);
	}
	
}

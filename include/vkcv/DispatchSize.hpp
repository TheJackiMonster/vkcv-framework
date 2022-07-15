#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/DispatchSize.hpp
 * @brief Class to handle dispatch sizes.
 */
 
#include <array>
#include <vulkan/vulkan.hpp>

namespace vkcv {
	
	class DispatchSize final {
	private:
		std::array<uint32_t, 3> m_Dispatch;
		
	public:
		DispatchSize(uint32_t count);
		
		DispatchSize(uint32_t dimensionX, uint32_t dimentionY, uint32_t dimensionZ = 1);
		
		DispatchSize(const DispatchSize& other) = default;
		DispatchSize(DispatchSize&& other) = default;
		
		~DispatchSize() = default;
		
		DispatchSize& operator=(const DispatchSize& other) = default;
		DispatchSize& operator=(DispatchSize&& other) = default;
		
		[[nodiscard]]
		const uint32_t* data() const;
		
		[[nodiscard]]
		uint32_t operator[](size_t index) const;
		
		[[nodiscard]]
		uint32_t x() const;
		
		[[nodiscard]]
		uint32_t y() const;
		
		[[nodiscard]]
		uint32_t z() const;
		
	};
	
	[[nodiscard]]
	DispatchSize dispatchInvocations(DispatchSize globalInvocations, DispatchSize groupSize);
	
}

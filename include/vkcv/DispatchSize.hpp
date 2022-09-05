#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/DispatchSize.hpp
 * @brief Class to handle dispatch sizes.
 */
 
#include <array>
#include <vulkan/vulkan.hpp>

namespace vkcv {
	
	/**
	 * @brief Class representing a dispatch size to invoke a compute pipeline with.
	 */
	class DispatchSize final {
	private:
		std::array<uint32_t, 3> m_Dispatch;
		
	public:
		/**
		 * Implicit constructor to convert an unsigned integer to a
		 * one dimensional dispatch size.
		 *
		 * @param[in] count Count of invocations
		 */
		DispatchSize(uint32_t count);
		
		/**
		 * Explicit constructor to create a dispatch size with two or
		 * three dimensions setting each value specifically.
		 *
		 * @param[in] dimensionX Size of X dimension
		 * @param[in] dimentionY Size of Y dimension
		 * @param[in] dimensionZ Size of Z dimension (optional)
		 */
		DispatchSize(uint32_t dimensionX, uint32_t dimentionY, uint32_t dimensionZ = 1);
		
		DispatchSize(const DispatchSize& other) = default;
		DispatchSize(DispatchSize&& other) = default;
		
		~DispatchSize() = default;
		
		DispatchSize& operator=(const DispatchSize& other) = default;
		DispatchSize& operator=(DispatchSize&& other) = default;
		
		/**
		 * Returns the raw data of the dispatch size as readonly unsigned
		 * integer pointer.
		 *
		 * @return Pointer to data
		 */
		[[nodiscard]]
		const uint32_t* data() const;
		
		/**
		 * Returns the size value of the dispatch size by a given index.
		 *
		 * @param[in] index Size index
		 * @return Size value by index
		 */
		[[nodiscard]]
		uint32_t operator[](size_t index) const;
		
		/**
		 * Returns the value for the X dimension of the dispatch size.
		 *
		 * @return Size of X dimension
		 */
		[[nodiscard]]
		uint32_t x() const;
		
		/**
		 * Returns the value for the Y dimension of the dispatch size.
		 *
		 * @return Size of Y dimension
		 */
		[[nodiscard]]
		uint32_t y() const;
		
		/**
		 * Returns the value for the Z dimension of the dispatch size.
		 *
		 * @return Size of Z dimension
		 */
		[[nodiscard]]
		uint32_t z() const;
		
		/**
		 * Checks whether the dispatch size is valid for compute shader
		 * invocations and returns the result as boolean value.
		 *
		 * @return True if the dispatch size is valid, otherwise false.
		 */
		bool check() const;
		
	};
	
	/**
	 * Returns the proper dispatch size by dividing a global amount of invocations
	 * as three dimensional dispatch size into invocations of a fixed group size
	 * for the used work groups of the compute shader.
	 *
	 * This function will generate over fitted results to make sure all global
	 * invocations get computed. So make sure the used compute shader handles those
	 * additional invocations out of bounds from the original global invocations.
	 *
	 * @param[in] globalInvocations Size of planned global invocations
	 * @param[in] groupSize Size of work group in compute stage
	 * @return Dispatch size respecting the actual work group size
	 */
	[[nodiscard]]
	DispatchSize dispatchInvocations(DispatchSize globalInvocations, DispatchSize groupSize);
	
}

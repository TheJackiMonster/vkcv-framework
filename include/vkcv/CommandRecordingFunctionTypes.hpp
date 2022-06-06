#pragma once
/**
 * @authors Alexander Gauggel, Tobias Frisch
 * @file vkcv/CommandRecordingFunctionTypes.hpp
 * @brief Abstract function types to handle command recording.
 */

#include <vulkan/vulkan.hpp>

#include "Event.hpp"

namespace vkcv {
	
	/**
	 * @brief Function to be called for recording a command buffer.
	 */
	typedef typename event_function<const vk::CommandBuffer&>::type RecordCommandFunction;
	
	/**
	 * @brief Function to be called after finishing a given process.
	 */
	typedef typename event_function<>::type FinishCommandFunction;
	
}
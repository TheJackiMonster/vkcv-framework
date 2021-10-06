#pragma once
/**
 * @authors Alexander Gauggel, Tobias Frisch
 * @file vkcv/CommandRecordingFunctionTypes.hpp
 * @brief Abstract function types to handle command recording.
 */

#include <vulkan/vulkan.hpp>

#include "Event.hpp"

namespace vkcv {
	
	typedef typename event_function<const vk::CommandBuffer&>::type RecordCommandFunction;
	typedef typename event_function<>::type FinishCommandFunction;
	
}
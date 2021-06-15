#pragma once
#include "vkcv/Event.hpp"
#include <vulkan/vulkan.hpp>

namespace vkcv {
	typedef typename event_function<const vk::CommandBuffer&>::type RecordCommandFunction;
	typedef typename event_function<>::type FinishCommandFunction;
}
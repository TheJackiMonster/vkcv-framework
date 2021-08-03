#pragma once
#include "vulkan/vulkan.hpp"

namespace AppConfig{
	const int           defaultWindowWidth  = 1280;
	const int           defaultWindowHeight = 720;
	const vk::Format    depthBufferFormat   = vk::Format::eD32Sfloat;
}
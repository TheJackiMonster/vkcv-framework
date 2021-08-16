#pragma once
#include "vulkan/vulkan.hpp"

namespace AppConfig{
	const int           defaultWindowWidth  = 1280;
	const int           defaultWindowHeight = 720;
	const vk::Format    depthBufferFormat   = vk::Format::eD32Sfloat;
	const vk::Format    colorBufferFormat   = vk::Format::eB10G11R11UfloatPack32;
	const vk::Format    motionBufferFormat  = vk::Format::eR16G16Sfloat;
    const uint32_t      maxMotionTileSize   = 8;
}
#pragma once
#include "vulkan/vulkan.hpp"

namespace MotionBlurConfig {
	const vk::Format    motionVectorTileFormat  = vk::Format::eR16G16Sfloat;
	const vk::Format    outputColorFormat       = vk::Format::eB10G11R11UfloatPack32;
	const uint32_t      maxMotionTileSize       = 20;	// must match "motionTileSize" in motionVectorMax.comp

	// small mouse movements are restricted to pixel level and therefore quite unprecise
	// therefore extrapolating movement at high framerates results in big jerky movements
	// this results in wide sudden motion blur, which looks quite bad
	// as a workaround the time scale is limited to a maximum value
	const float timeScaleMax = 1.f / 60;
}
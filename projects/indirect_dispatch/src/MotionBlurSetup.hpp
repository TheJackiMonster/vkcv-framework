#pragma once
#include <vkcv/Core.hpp>

struct MotionBlurRenderTargets {
	vkcv::ImageHandle outputColor;
	vkcv::ImageHandle motionMax;
	vkcv::ImageHandle motionMaxNeighbourhood;
	vkcv::ImageHandle motionMin;
	vkcv::ImageHandle motionMinNeighbourhood;
};

namespace MotionBlurSetup {
	MotionBlurRenderTargets createRenderTargets(const uint32_t width, const uint32_t height, vkcv::Core& core);
}
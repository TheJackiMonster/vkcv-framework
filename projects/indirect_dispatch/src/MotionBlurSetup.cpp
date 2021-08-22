#include "MotionBlurSetup.hpp"
#include "MotionBlurConfig.hpp"

namespace MotionBlurSetup {

MotionBlurRenderTargets createRenderTargets(const uint32_t width, const uint32_t height, vkcv::Core& core) {

	MotionBlurRenderTargets targets;

	// divide and ceil to int
	const uint32_t motionMaxWidth  = (width  + (MotionBlurConfig::maxMotionTileSize - 1)) / MotionBlurConfig::maxMotionTileSize;
	const uint32_t motionMaxheight = (height + (MotionBlurConfig::maxMotionTileSize - 1)) / MotionBlurConfig::maxMotionTileSize;

	targets.motionMax = core.createImage(
		MotionBlurConfig::motionVectorTileFormat,
		motionMaxWidth,
		motionMaxheight,
		1,
		false,
		true).getHandle();

	targets.motionMaxNeighbourhood = core.createImage(
		MotionBlurConfig::motionVectorTileFormat,
		motionMaxWidth,
		motionMaxheight,
		1,
		false,
		true).getHandle();

	targets.motionMin = core.createImage(
		MotionBlurConfig::motionVectorTileFormat,
		motionMaxWidth,
		motionMaxheight,
		1,
		false,
		true).getHandle();

	targets.motionMinNeighbourhood = core.createImage(
		MotionBlurConfig::motionVectorTileFormat,
		motionMaxWidth,
		motionMaxheight,
		1,
		false,
		true).getHandle();

	targets.outputColor = core.createImage(
		MotionBlurConfig::outputColorFormat,
		width,
		height,
		1,
		false,
		true).getHandle();

	return targets;
}

}	// namespace MotionBlurSetup
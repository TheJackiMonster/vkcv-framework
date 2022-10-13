#include "MotionBlurSetup.hpp"
#include "MotionBlurConfig.hpp"

namespace MotionBlurSetup {

MotionBlurRenderTargets createRenderTargets(const uint32_t width, const uint32_t height, vkcv::Core& core) {

	MotionBlurRenderTargets targets;

	// divide and ceil to int
	const uint32_t motionMaxWidth  = (width  + (MotionBlurConfig::maxMotionTileSize - 1)) / MotionBlurConfig::maxMotionTileSize;
	const uint32_t motionMaxHeight = (height + (MotionBlurConfig::maxMotionTileSize - 1)) / MotionBlurConfig::maxMotionTileSize;

	vkcv::ImageConfig targetConfig (motionMaxWidth, motionMaxHeight);
	targetConfig.setSupportingStorage(true);
	
	targets.motionMax = core.createImage(
		MotionBlurConfig::motionVectorTileFormat,
		targetConfig
	);

	targets.motionMaxNeighbourhood = core.createImage(
		MotionBlurConfig::motionVectorTileFormat,
		targetConfig
	);

	targets.motionMin = core.createImage(
		MotionBlurConfig::motionVectorTileFormat,
		targetConfig
	);

	targets.motionMinNeighbourhood = core.createImage(
		MotionBlurConfig::motionVectorTileFormat,
		targetConfig
	);
	
	vkcv::ImageConfig outputConfig (width, height);
	outputConfig.setSupportingStorage(true);

	targets.outputColor = core.createImage(
		MotionBlurConfig::outputColorFormat,
		outputConfig
	);

	return targets;
}

}	// namespace MotionBlurSetup
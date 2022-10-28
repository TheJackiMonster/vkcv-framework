#include "MotionBlur.hpp"
#include "MotionBlurConfig.hpp"
#include "MotionBlurSetup.hpp"

#include <vkcv/Buffer.hpp>
#include <vkcv/Sampler.hpp>

#include <array>

bool MotionBlur::initialize(vkcv::Core* corePtr, const uint32_t targetWidth, const uint32_t targetHeight) {

	if (!corePtr) {
		vkcv_log(vkcv::LogLevel::ERROR, "MotionBlur got invalid corePtr")
		return false;
	}

	m_core = corePtr;

	if (!loadComputePass(*m_core, "assets/shaders/motionBlur.comp", &m_motionBlurPass))
		return false;

	if (!loadComputePass(*m_core, "assets/shaders/motionVectorMinMax.comp", &m_motionVectorMinMaxPass))
		return false;

	if (!loadComputePass(*m_core, "assets/shaders/motionVectorMinMaxNeighbourhood.comp", &m_motionVectorMinMaxNeighbourhoodPass))
		return false;

	if (!loadComputePass(*m_core, "assets/shaders/motionVectorVisualisation.comp", &m_motionVectorVisualisationPass))
		return false;

	if (!loadComputePass(*m_core, "assets/shaders/motionBlurColorCopy.comp", &m_colorCopyPass))
		return false;

	if (!loadComputePass(*m_core, "assets/shaders/motionBlurTileClassification.comp", &m_tileClassificationPass))
		return false;

	if (!loadComputePass(*m_core, "assets/shaders/motionBlurWorkTileReset.comp", &m_tileResetPass))
		return false;

	if (!loadComputePass(*m_core, "assets/shaders/motionBlurTileClassificationVis.comp", &m_tileVisualisationPass))
		return false;

	if (!loadComputePass(*m_core, "assets/shaders/motionBlurFastPath.comp", &m_motionBlurFastPathPass))
		return false;

	// work tile buffers and descriptors
	const uint32_t workTileBufferSize = static_cast<uint32_t>(2 * sizeof(uint32_t)) * (3 +
		((MotionBlurConfig::maxWidth + MotionBlurConfig::maxMotionTileSize - 1) / MotionBlurConfig::maxMotionTileSize) *
		((MotionBlurConfig::maxHeight + MotionBlurConfig::maxMotionTileSize - 1) / MotionBlurConfig::maxMotionTileSize));

	m_copyPathWorkTileBuffer = vkcv::buffer<uint32_t>(
		*m_core,
		vkcv::BufferType::INDIRECT,
		workTileBufferSize, 
		vkcv::BufferMemoryType::DEVICE_LOCAL).getHandle();

	m_fullPathWorkTileBuffer = vkcv::buffer<uint32_t>(
		*m_core,
		vkcv::BufferType::INDIRECT,
		workTileBufferSize, 
		vkcv::BufferMemoryType::DEVICE_LOCAL).getHandle();

	m_fastPathWorkTileBuffer = vkcv::buffer<uint32_t>(
		*m_core,
		vkcv::BufferType::INDIRECT,
		workTileBufferSize,
		vkcv::BufferMemoryType::DEVICE_LOCAL).getHandle();

	vkcv::DescriptorWrites tileResetDescriptorWrites;
	tileResetDescriptorWrites.writeStorageBuffer(
			0, m_fullPathWorkTileBuffer
	).writeStorageBuffer(
			1, m_copyPathWorkTileBuffer
	).writeStorageBuffer(
			2, m_fastPathWorkTileBuffer
	);

	m_core->writeDescriptorSet(m_tileResetPass.descriptorSet, tileResetDescriptorWrites);

	m_renderTargets = MotionBlurSetup::createRenderTargets(targetWidth, targetHeight, *m_core);

	m_nearestSampler = vkcv::samplerNearest(*m_core, true);
	
	return true;
}

void MotionBlur::setResolution(const uint32_t targetWidth, const uint32_t targetHeight) {
	m_renderTargets = MotionBlurSetup::createRenderTargets(targetWidth, targetHeight, *m_core);
}

vkcv::ImageHandle MotionBlur::render(
	const vkcv::CommandStreamHandle cmdStream,
	const vkcv::ImageHandle         motionBufferFullRes,
	const vkcv::ImageHandle         colorBuffer,
	const vkcv::ImageHandle         depthBuffer,
	const eMotionBlurMode           mode,
	const float                     cameraNear,
	const float                     cameraFar,
	const float                     deltaTimeSeconds,
	const float                     cameraShutterSpeedInverse,
	const float                     motionTileOffsetLength,
	const float                     fastPathThreshold) {

	computeMotionTiles(cmdStream, motionBufferFullRes);

	m_core->recordComputeDispatchToCmdStream(
		cmdStream,
		m_tileResetPass.pipeline,
		1,
		{ vkcv::useDescriptorSet(0, m_tileResetPass.descriptorSet) },
		vkcv::PushConstants(0)
	);

	m_core->recordBufferMemoryBarrier(cmdStream, m_fullPathWorkTileBuffer);
	m_core->recordBufferMemoryBarrier(cmdStream, m_copyPathWorkTileBuffer);
	m_core->recordBufferMemoryBarrier(cmdStream, m_fastPathWorkTileBuffer);

	// work tile classification
	vkcv::DescriptorWrites tileClassificationDescriptorWrites;
	tileClassificationDescriptorWrites.writeSampledImage(
			0, m_renderTargets.motionMaxNeighbourhood
	).writeSampledImage(
			1, m_renderTargets.motionMinNeighbourhood
	);
	
	tileClassificationDescriptorWrites.writeSampler(2, m_nearestSampler);
	tileClassificationDescriptorWrites.writeStorageBuffer(
			3, m_fullPathWorkTileBuffer
	).writeStorageBuffer(
			4, m_copyPathWorkTileBuffer
	).writeStorageBuffer(
			5, m_fastPathWorkTileBuffer
	);

	m_core->writeDescriptorSet(m_tileClassificationPass.descriptorSet, tileClassificationDescriptorWrites);

	const auto tileClassificationDispatch = vkcv::dispatchInvocations(
			vkcv::DispatchSize(
					m_core->getImageWidth(m_renderTargets.motionMaxNeighbourhood),
					m_core->getImageHeight(m_renderTargets.motionMaxNeighbourhood)
			),
			vkcv::DispatchSize(8, 8)
	);

	struct ClassificationConstants {
		uint32_t    width;
		uint32_t    height;
		float       fastPathThreshold;
	};
	ClassificationConstants classificationConstants;
	classificationConstants.width               = m_core->getImageWidth(m_renderTargets.outputColor);
	classificationConstants.height              = m_core->getImageHeight(m_renderTargets.outputColor);
	classificationConstants.fastPathThreshold   = fastPathThreshold;

	vkcv::PushConstants classificationPushConstants = vkcv::pushConstants<ClassificationConstants>();
    classificationPushConstants.appendDrawcall(classificationConstants);

	m_core->prepareImageForSampling(cmdStream, m_renderTargets.motionMaxNeighbourhood);
	m_core->prepareImageForSampling(cmdStream, m_renderTargets.motionMinNeighbourhood);

	m_core->recordComputeDispatchToCmdStream(
		cmdStream,
		m_tileClassificationPass.pipeline,
		tileClassificationDispatch,
		{ vkcv::useDescriptorSet(0, m_tileClassificationPass.descriptorSet) },
		classificationPushConstants);

	m_core->recordBufferMemoryBarrier(cmdStream, m_fullPathWorkTileBuffer);
	m_core->recordBufferMemoryBarrier(cmdStream, m_copyPathWorkTileBuffer);
	m_core->recordBufferMemoryBarrier(cmdStream, m_fastPathWorkTileBuffer);

	vkcv::DescriptorWrites motionBlurDescriptorWrites;
	motionBlurDescriptorWrites.writeSampledImage(
			0, colorBuffer
	).writeSampledImage(
			1, depthBuffer
	).writeSampledImage(
			2, motionBufferFullRes
	).writeSampledImage(
			3, m_renderTargets.motionMaxNeighbourhood
	);
	
	motionBlurDescriptorWrites.writeSampler(4, m_nearestSampler);
	motionBlurDescriptorWrites.writeStorageImage(5, m_renderTargets.outputColor);
	motionBlurDescriptorWrites.writeStorageBuffer(6, m_fullPathWorkTileBuffer);

	m_core->writeDescriptorSet(m_motionBlurPass.descriptorSet, motionBlurDescriptorWrites);

	vkcv::DescriptorWrites colorCopyDescriptorWrites;
	colorCopyDescriptorWrites.writeSampledImage(0, colorBuffer);
	colorCopyDescriptorWrites.writeSampler(1, m_nearestSampler);
	colorCopyDescriptorWrites.writeStorageImage(2, m_renderTargets.outputColor);
	colorCopyDescriptorWrites.writeStorageBuffer(3, m_copyPathWorkTileBuffer);

	m_core->writeDescriptorSet(m_colorCopyPass.descriptorSet, colorCopyDescriptorWrites);


	vkcv::DescriptorWrites fastPathDescriptorWrites;
	fastPathDescriptorWrites.writeSampledImage(
			0, colorBuffer
	).writeSampledImage(
			1, m_renderTargets.motionMaxNeighbourhood
	);
	
	fastPathDescriptorWrites.writeSampler(2, m_nearestSampler);
	fastPathDescriptorWrites.writeStorageImage(3, m_renderTargets.outputColor);
	fastPathDescriptorWrites.writeStorageBuffer(4, m_fastPathWorkTileBuffer);

	m_core->writeDescriptorSet(m_motionBlurFastPathPass.descriptorSet, fastPathDescriptorWrites);

	// must match layout in "motionBlur.comp"
	struct MotionBlurConstantData {
		float motionFactor;
		float cameraNearPlane;
		float cameraFarPlane;
		float motionTileOffsetLength;
	};
	MotionBlurConstantData motionBlurConstantData;

	const float deltaTimeMotionBlur = deltaTimeSeconds;

	motionBlurConstantData.motionFactor             = 1 / (deltaTimeMotionBlur * cameraShutterSpeedInverse);
	motionBlurConstantData.cameraNearPlane          = cameraNear;
	motionBlurConstantData.cameraFarPlane           = cameraFar;
	motionBlurConstantData.motionTileOffsetLength   = motionTileOffsetLength;

	vkcv::PushConstants motionBlurPushConstants = vkcv::pushConstants<MotionBlurConstantData>();
	motionBlurPushConstants.appendDrawcall(motionBlurConstantData);

	struct FastPathConstants {
		float motionFactor;
	};
	FastPathConstants fastPathConstants;
	fastPathConstants.motionFactor = motionBlurConstantData.motionFactor;

	vkcv::PushConstants fastPathPushConstants = vkcv::pushConstants<FastPathConstants>();
	fastPathPushConstants.appendDrawcall(fastPathConstants);

	m_core->prepareImageForStorage(cmdStream, m_renderTargets.outputColor);
	m_core->prepareImageForSampling(cmdStream, colorBuffer);
	m_core->prepareImageForSampling(cmdStream, depthBuffer);
	m_core->prepareImageForSampling(cmdStream, m_renderTargets.motionMaxNeighbourhood);

	if (mode == eMotionBlurMode::Default) {
		m_core->recordComputeIndirectDispatchToCmdStream(
			cmdStream,
			m_motionBlurPass.pipeline,
			m_fullPathWorkTileBuffer,
			0,
			{ vkcv::useDescriptorSet(0, m_motionBlurPass.descriptorSet) },
			motionBlurPushConstants);

		m_core->recordComputeIndirectDispatchToCmdStream(
			cmdStream,
			m_colorCopyPass.pipeline,
			m_copyPathWorkTileBuffer,
			0,
			{ vkcv::useDescriptorSet(0, m_colorCopyPass.descriptorSet) },
			vkcv::PushConstants(0));

		m_core->recordComputeIndirectDispatchToCmdStream(
			cmdStream,
			m_motionBlurFastPathPass.pipeline,
			m_fastPathWorkTileBuffer,
			0,
			{ vkcv::useDescriptorSet(0, m_motionBlurFastPathPass.descriptorSet) },
			fastPathPushConstants);
	}
	else if(mode == eMotionBlurMode::Disabled) {
		return colorBuffer;
	}
	else if (mode == eMotionBlurMode::TileVisualisation) {

		vkcv::DescriptorWrites visualisationDescriptorWrites;
		visualisationDescriptorWrites.writeSampledImage(0, colorBuffer);
		visualisationDescriptorWrites.writeSampler(1, m_nearestSampler);
		visualisationDescriptorWrites.writeStorageImage(2, m_renderTargets.outputColor);
		
		visualisationDescriptorWrites.writeStorageBuffer(
				3, m_fullPathWorkTileBuffer
		).writeStorageBuffer(
				4, m_copyPathWorkTileBuffer
		).writeStorageBuffer(
				5, m_fastPathWorkTileBuffer
		);

		m_core->writeDescriptorSet(m_tileVisualisationPass.descriptorSet, visualisationDescriptorWrites);

		const uint32_t tileCount = 
			(m_core->getImageWidth(m_renderTargets.outputColor)  + MotionBlurConfig::maxMotionTileSize - 1) / MotionBlurConfig::maxMotionTileSize * 
			(m_core->getImageHeight(m_renderTargets.outputColor) + MotionBlurConfig::maxMotionTileSize - 1) / MotionBlurConfig::maxMotionTileSize;

		m_core->recordComputeDispatchToCmdStream(
			cmdStream,
			m_tileVisualisationPass.pipeline,
			tileCount,
			{ vkcv::useDescriptorSet(0, m_tileVisualisationPass.descriptorSet) },
			vkcv::PushConstants(0));
	}
	else {
		vkcv_log(vkcv::LogLevel::ERROR, "Unknown eMotionBlurMode enum option");
		return colorBuffer;
	}

	return m_renderTargets.outputColor;
}

vkcv::ImageHandle MotionBlur::renderMotionVectorVisualisation(
	const vkcv::CommandStreamHandle         cmdStream,
	const vkcv::ImageHandle                 motionBuffer,
	const eMotionVectorVisualisationMode    mode,
	const float                             velocityRange) {

	computeMotionTiles(cmdStream, motionBuffer);

	vkcv::ImageHandle visualisationInput;
	if (     mode == eMotionVectorVisualisationMode::FullResolution)
		visualisationInput = motionBuffer;
	else if (mode == eMotionVectorVisualisationMode::MaxTile)
		visualisationInput = m_renderTargets.motionMax;
	else if (mode == eMotionVectorVisualisationMode::MaxTileNeighbourhood)
		visualisationInput = m_renderTargets.motionMaxNeighbourhood;
	else if (mode == eMotionVectorVisualisationMode::MinTile)
		visualisationInput = m_renderTargets.motionMin;
	else if (mode == eMotionVectorVisualisationMode::MinTileNeighbourhood)
		visualisationInput = m_renderTargets.motionMinNeighbourhood;
	else if (mode == eMotionVectorVisualisationMode::None) {
		vkcv_log(vkcv::LogLevel::ERROR, "renderMotionVectorVisualisation called with visualisation mode 'None'");
		return motionBuffer;
	}
	else {
		vkcv_log(vkcv::LogLevel::ERROR, "Unknown eDebugView enum value");
		return motionBuffer;
	}

	vkcv::DescriptorWrites motionVectorVisualisationDescriptorWrites;
	motionVectorVisualisationDescriptorWrites.writeSampledImage(0, visualisationInput);
	motionVectorVisualisationDescriptorWrites.writeSampler(1, m_nearestSampler);
	motionVectorVisualisationDescriptorWrites.writeStorageImage(2, m_renderTargets.outputColor);

	m_core->writeDescriptorSet(
		m_motionVectorVisualisationPass.descriptorSet,
		motionVectorVisualisationDescriptorWrites);

	m_core->prepareImageForSampling(cmdStream, visualisationInput);
	m_core->prepareImageForStorage(cmdStream, m_renderTargets.outputColor);

	vkcv::PushConstants motionVectorVisualisationPushConstants = vkcv::pushConstants<float>();
	motionVectorVisualisationPushConstants.appendDrawcall(velocityRange);

	const auto dispatchSizes = vkcv::dispatchInvocations(
			vkcv::DispatchSize(
					m_core->getImageWidth(m_renderTargets.outputColor),
					m_core->getImageHeight(m_renderTargets.outputColor)
			),
			vkcv::DispatchSize(8, 8)
	);

	m_core->recordComputeDispatchToCmdStream(
		cmdStream,
		m_motionVectorVisualisationPass.pipeline,
		dispatchSizes,
		{ vkcv::useDescriptorSet(0, m_motionVectorVisualisationPass.descriptorSet) },
		motionVectorVisualisationPushConstants);

	return m_renderTargets.outputColor;
}

void MotionBlur::computeMotionTiles(
	const vkcv::CommandStreamHandle cmdStream,
	const vkcv::ImageHandle         motionBufferFullRes) {

	// motion vector min max tiles
	vkcv::DescriptorWrites motionVectorMaxTilesDescriptorWrites;
	motionVectorMaxTilesDescriptorWrites.writeSampledImage(0, motionBufferFullRes);
	motionVectorMaxTilesDescriptorWrites.writeSampler(1, m_nearestSampler);
	motionVectorMaxTilesDescriptorWrites.writeStorageImage(
			2, m_renderTargets.motionMax
	).writeStorageImage(
			3, m_renderTargets.motionMin
	);

	m_core->writeDescriptorSet(m_motionVectorMinMaxPass.descriptorSet, motionVectorMaxTilesDescriptorWrites);

	m_core->prepareImageForSampling(cmdStream, motionBufferFullRes);
	m_core->prepareImageForStorage(cmdStream, m_renderTargets.motionMax);
	m_core->prepareImageForStorage(cmdStream, m_renderTargets.motionMin);

	const auto motionTileDispatchCounts = vkcv::dispatchInvocations(
			vkcv::DispatchSize(
					m_core->getImageWidth( m_renderTargets.motionMax),
					m_core->getImageHeight(m_renderTargets.motionMax)
			),
			vkcv::DispatchSize(8, 8)
	);

	m_core->recordComputeDispatchToCmdStream(
		cmdStream,
		m_motionVectorMinMaxPass.pipeline,
		motionTileDispatchCounts,
		{ vkcv::useDescriptorSet(0, m_motionVectorMinMaxPass.descriptorSet) },
		vkcv::PushConstants(0));

	// motion vector min max neighbourhood
	vkcv::DescriptorWrites motionVectorMaxNeighbourhoodDescriptorWrites;
	motionVectorMaxNeighbourhoodDescriptorWrites.writeSampledImage(
			0, m_renderTargets.motionMax
	).writeSampledImage(
			1, m_renderTargets.motionMin
	);
	
	motionVectorMaxNeighbourhoodDescriptorWrites.writeSampler(2, m_nearestSampler);
	motionVectorMaxNeighbourhoodDescriptorWrites.writeStorageImage(
			3, m_renderTargets.motionMaxNeighbourhood
	).writeStorageImage(
			4, m_renderTargets.motionMinNeighbourhood
	);

	m_core->writeDescriptorSet(m_motionVectorMinMaxNeighbourhoodPass.descriptorSet, motionVectorMaxNeighbourhoodDescriptorWrites);

	m_core->prepareImageForSampling(cmdStream, m_renderTargets.motionMax);
	m_core->prepareImageForSampling(cmdStream, m_renderTargets.motionMin);

	m_core->prepareImageForStorage(cmdStream, m_renderTargets.motionMaxNeighbourhood);
	m_core->prepareImageForStorage(cmdStream, m_renderTargets.motionMinNeighbourhood);

	m_core->recordComputeDispatchToCmdStream(
		cmdStream,
		m_motionVectorMinMaxNeighbourhoodPass.pipeline,
		motionTileDispatchCounts,
		{ vkcv::useDescriptorSet(0, m_motionVectorMinMaxNeighbourhoodPass.descriptorSet) },
		vkcv::PushConstants(0));
}
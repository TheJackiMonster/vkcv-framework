#include "MotionBlur.hpp"
#include "MotionBlurConfig.hpp"
#include "MotionBlurSetup.hpp"
#include <array>

std::array<uint32_t, 3> computeFullscreenDispatchSize(
	const uint32_t imageWidth,
	const uint32_t imageHeight,
	const uint32_t localGroupSize) {

	// optimized divide and ceil
	return std::array<uint32_t, 3>{
		static_cast<uint32_t>(imageWidth  + (localGroupSize - 1) / localGroupSize),
		static_cast<uint32_t>(imageHeight + (localGroupSize - 1) / localGroupSize),
		static_cast<uint32_t>(1) };
}

bool MotionBlur::initialize(vkcv::Core* corePtr, const uint32_t targetWidth, const uint32_t targetHeight) {

	if (!corePtr) {
		vkcv_log(vkcv::LogLevel::ERROR, "MotionBlur got invalid corePtr")
		return false;
	}

	m_core = corePtr;

	if (!loadComputePass(*m_core, "resources/shaders/motionBlur.comp", &m_motionBlurPass))
		return false;

	if (!loadComputePass(*m_core, "resources/shaders/motionVectorMax.comp", &m_motionVectorMaxPass))
		return false;

	if (!loadComputePass(*m_core, "resources/shaders/motionVectorMaxNeighbourhood.comp", &m_motionVectorMaxNeighbourhoodPass))
		return false;

	if (!loadComputePass(*m_core, "resources/shaders/motionVectorVisualisation.comp", &m_motionVectorVisualisationPass))
		return false;

	m_renderTargets = MotionBlurSetup::createRenderTargets(targetWidth, targetHeight, *m_core);

	m_nearestSampler = m_core->createSampler(
		vkcv::SamplerFilterType::NEAREST,
		vkcv::SamplerFilterType::NEAREST,
		vkcv::SamplerMipmapMode::NEAREST,
		vkcv::SamplerAddressMode::CLAMP_TO_EDGE);
}

void MotionBlur::setResolution(const uint32_t targetWidth, const uint32_t targetHeight) {
	m_renderTargets = MotionBlurSetup::createRenderTargets(targetWidth, targetHeight, *m_core);
}

vkcv::ImageHandle MotionBlur::render(
	const vkcv::CommandStreamHandle cmdStream,
	const vkcv::ImageHandle         motionBufferFullRes,
	const vkcv::ImageHandle         colorBuffer,
	const vkcv::ImageHandle         depthBuffer,
	const eMotionVectorMode         motionVectorMode,
	const float                     cameraNear,
	const float                     cameraFar,
	const float                     deltaTimeSeconds,
	const float                     cameraShutterSpeedInverse) {

	computeMotionTiles(cmdStream, motionBufferFullRes);

	// usually this is the neighbourhood max, but other modes can be used for comparison/debugging
	vkcv::ImageHandle inputMotionTiles;
	if (motionVectorMode == eMotionVectorMode::FullResolution)
		inputMotionTiles = motionBufferFullRes;
	else if (motionVectorMode == eMotionVectorMode::MaxTile)
		inputMotionTiles = m_renderTargets.motionMax;
	else if (motionVectorMode == eMotionVectorMode::MaxTileNeighbourhood)
		inputMotionTiles = m_renderTargets.motionMaxNeighbourhood;
	else {
		vkcv_log(vkcv::LogLevel::ERROR, "Unknown eMotionInput enum value");
		inputMotionTiles = m_renderTargets.motionMaxNeighbourhood;
	}

	vkcv::DescriptorWrites motionBlurDescriptorWrites;
	motionBlurDescriptorWrites.sampledImageWrites = {
		vkcv::SampledImageDescriptorWrite(0, colorBuffer),
		vkcv::SampledImageDescriptorWrite(1, depthBuffer),
		vkcv::SampledImageDescriptorWrite(2, motionBufferFullRes),
		vkcv::SampledImageDescriptorWrite(3, inputMotionTiles) };
	motionBlurDescriptorWrites.samplerWrites = {
		vkcv::SamplerDescriptorWrite(4, m_nearestSampler) };
	motionBlurDescriptorWrites.storageImageWrites = {
		vkcv::StorageImageDescriptorWrite(5, m_renderTargets.outputColor) };

	m_core->writeDescriptorSet(m_motionBlurPass.descriptorSet, motionBlurDescriptorWrites);

	// must match layout in "motionBlur.comp"
	struct MotionBlurConstantData {
		float motionFactor;
		float cameraNearPlane;
		float cameraFarPlane;
	};
	MotionBlurConstantData motionBlurConstantData;

	const float deltaTimeMotionBlur = deltaTimeSeconds;

	motionBlurConstantData.motionFactor     = 1 / (deltaTimeMotionBlur * cameraShutterSpeedInverse);
	motionBlurConstantData.cameraNearPlane  = cameraNear;
	motionBlurConstantData.cameraFarPlane   = cameraFar;

	vkcv::PushConstants motionBlurPushConstants(sizeof(motionBlurConstantData));
	motionBlurPushConstants.appendDrawcall(motionBlurConstantData);

	m_core->prepareImageForStorage(cmdStream, m_renderTargets.outputColor);
	m_core->prepareImageForSampling(cmdStream, colorBuffer);
	m_core->prepareImageForSampling(cmdStream, depthBuffer);
	m_core->prepareImageForSampling(cmdStream, inputMotionTiles);

	const auto fullscreenDispatchSizes = computeFullscreenDispatchSize(
		m_core->getImageWidth(m_renderTargets.outputColor),
		m_core->getImageHeight(m_renderTargets.outputColor),
		8);

	m_core->recordComputeDispatchToCmdStream(
		cmdStream,
		m_motionBlurPass.pipeline,
		fullscreenDispatchSizes.data(),
		{ vkcv::DescriptorSetUsage(0, m_core->getDescriptorSet(m_motionBlurPass.descriptorSet).vulkanHandle) },
		motionBlurPushConstants);

	return m_renderTargets.outputColor;
}

vkcv::ImageHandle MotionBlur::renderMotionVectorVisualisation(
	const vkcv::CommandStreamHandle cmdStream,
	const vkcv::ImageHandle         motionBuffer,
	const eMotionVectorMode         debugView,
	const float                     velocityRange) {

	computeMotionTiles(cmdStream, motionBuffer);

	vkcv::ImageHandle visualisationInput;
	if (     debugView == eMotionVectorMode::FullResolution)
		visualisationInput = motionBuffer;
	else if (debugView == eMotionVectorMode::MaxTile)
		visualisationInput = m_renderTargets.motionMax;
	else if (debugView == eMotionVectorMode::MaxTileNeighbourhood)
		visualisationInput = m_renderTargets.motionMaxNeighbourhood;
	else {
		vkcv_log(vkcv::LogLevel::ERROR, "Unknown eDebugView enum value");
		return motionBuffer;
	}

	vkcv::DescriptorWrites motionVectorVisualisationDescriptorWrites;
	motionVectorVisualisationDescriptorWrites.sampledImageWrites = {
		vkcv::SampledImageDescriptorWrite(0, visualisationInput) };
	motionVectorVisualisationDescriptorWrites.samplerWrites = {
		vkcv::SamplerDescriptorWrite(1, m_nearestSampler) };
	motionVectorVisualisationDescriptorWrites.storageImageWrites = {
		vkcv::StorageImageDescriptorWrite(2, m_renderTargets.outputColor) };

	m_core->writeDescriptorSet(
		m_motionVectorVisualisationPass.descriptorSet,
		motionVectorVisualisationDescriptorWrites);

	m_core->prepareImageForSampling(cmdStream, visualisationInput);
	m_core->prepareImageForStorage(cmdStream, m_renderTargets.outputColor);

	vkcv::PushConstants motionVectorVisualisationPushConstants(sizeof(float));
	motionVectorVisualisationPushConstants.appendDrawcall(velocityRange);

	const auto dispatchSizes = computeFullscreenDispatchSize(
		m_core->getImageWidth(m_renderTargets.outputColor), 
		m_core->getImageHeight(m_renderTargets.outputColor), 
		8);

	m_core->recordComputeDispatchToCmdStream(
		cmdStream,
		m_motionVectorVisualisationPass.pipeline,
		dispatchSizes.data(),
		{ vkcv::DescriptorSetUsage(0, m_core->getDescriptorSet(m_motionVectorVisualisationPass.descriptorSet).vulkanHandle) },
		motionVectorVisualisationPushConstants);

	return m_renderTargets.outputColor;
}

void MotionBlur::computeMotionTiles(
	const vkcv::CommandStreamHandle cmdStream,
	const vkcv::ImageHandle         motionBufferFullRes) {

	// motion vector max tiles
	vkcv::DescriptorWrites motionVectorMaxTilesDescriptorWrites;
	motionVectorMaxTilesDescriptorWrites.sampledImageWrites = {
		vkcv::SampledImageDescriptorWrite(0, motionBufferFullRes) };
	motionVectorMaxTilesDescriptorWrites.samplerWrites = {
		vkcv::SamplerDescriptorWrite(1, m_nearestSampler) };
	motionVectorMaxTilesDescriptorWrites.storageImageWrites = {
		vkcv::StorageImageDescriptorWrite(2, m_renderTargets.motionMax) };

	m_core->writeDescriptorSet(m_motionVectorMaxPass.descriptorSet, motionVectorMaxTilesDescriptorWrites);

	m_core->prepareImageForSampling(cmdStream, motionBufferFullRes);
	m_core->prepareImageForStorage(cmdStream, m_renderTargets.motionMax);

	const std::array<uint32_t, 3> motionTileDispatchCounts = computeFullscreenDispatchSize(
		m_core->getImageWidth( m_renderTargets.motionMax),
		m_core->getImageHeight(m_renderTargets.motionMax),
		8);

	m_core->recordComputeDispatchToCmdStream(
		cmdStream,
		m_motionVectorMaxPass.pipeline,
		motionTileDispatchCounts.data(),
		{ vkcv::DescriptorSetUsage(0, m_core->getDescriptorSet(m_motionVectorMaxPass.descriptorSet).vulkanHandle) },
		vkcv::PushConstants(0));

	// motion vector max neighbourhood
	vkcv::DescriptorWrites motionVectorMaxNeighbourhoodDescriptorWrites;
	motionVectorMaxNeighbourhoodDescriptorWrites.sampledImageWrites = {
		vkcv::SampledImageDescriptorWrite(0, m_renderTargets.motionMax) };
	motionVectorMaxNeighbourhoodDescriptorWrites.samplerWrites = {
		vkcv::SamplerDescriptorWrite(1, m_nearestSampler) };
	motionVectorMaxNeighbourhoodDescriptorWrites.storageImageWrites = {
		vkcv::StorageImageDescriptorWrite(2, m_renderTargets.motionMaxNeighbourhood) };

	m_core->writeDescriptorSet(m_motionVectorMaxNeighbourhoodPass.descriptorSet, motionVectorMaxNeighbourhoodDescriptorWrites);

	m_core->prepareImageForSampling(cmdStream, m_renderTargets.motionMax);
	m_core->prepareImageForStorage(cmdStream, m_renderTargets.motionMaxNeighbourhood);

	m_core->recordComputeDispatchToCmdStream(
		cmdStream,
		m_motionVectorMaxNeighbourhoodPass.pipeline,
		motionTileDispatchCounts.data(),
		{ vkcv::DescriptorSetUsage(0, m_core->getDescriptorSet(m_motionVectorMaxNeighbourhoodPass.descriptorSet).vulkanHandle) },
		vkcv::PushConstants(0));
}
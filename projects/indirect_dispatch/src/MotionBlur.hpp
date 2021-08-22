#pragma once
#include <vkcv/Core.hpp>
#include "AppSetup.hpp"
#include "MotionBlurSetup.hpp"

// selection for motion blur input and visualisation
enum class eMotionVectorVisualisationMode : int {
	None                    = 0,
	FullResolution          = 1,
	MaxTile                 = 2,
	MaxTileNeighbourhood    = 3,
	MinTile                 = 4,
	MinTileNeighbourhood    = 5,
	OptionCount             = 6 };

static const char* MotionVectorVisualisationModeLabels[6] = {
	"None",
	"Full resolution",
	"Max tile",
	"Tile neighbourhood max",
	"Min Tile",
	"Tile neighbourhood min"};

enum class eMotionBlurMode : int {
	Default             = 0,
	Disabled            = 1,
	TileVisualisation   = 2,
	OptionCount         = 3 };

static const char* MotionBlurModeLabels[3] = {
	"Default",
	"Disabled",
	"Tile visualisation" };

class MotionBlur {
public:

	bool initialize(vkcv::Core* corePtr, const uint32_t targetWidth, const uint32_t targetHeight);
	void setResolution(const uint32_t targetWidth, const uint32_t targetHeight);

	vkcv::ImageHandle render(
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
		const float                     fastPathThreshold);

	vkcv::ImageHandle renderMotionVectorVisualisation(
		const vkcv::CommandStreamHandle         cmdStream,
		const vkcv::ImageHandle                 motionBuffer,
		const eMotionVectorVisualisationMode    mode,
		const float                             velocityRange);

private:
	// computes max per tile and neighbourhood tile max
	void computeMotionTiles(
		const vkcv::CommandStreamHandle cmdStream,
		const vkcv::ImageHandle         motionBufferFullRes);

	vkcv::Core* m_core;

	MotionBlurRenderTargets m_renderTargets;
	vkcv::SamplerHandle     m_nearestSampler;

	ComputePassHandles m_motionBlurPass;
	ComputePassHandles m_motionVectorMinMaxPass;
	ComputePassHandles m_motionVectorMinMaxNeighbourhoodPass;
	ComputePassHandles m_motionVectorVisualisationPass;
	ComputePassHandles m_colorCopyPass;
	ComputePassHandles m_tileClassificationPass;
	ComputePassHandles m_tileResetPass;
	ComputePassHandles m_tileVisualisationPass;
	ComputePassHandles m_motionBlurFastPathPass;

	vkcv::BufferHandle m_fullPathWorkTileBuffer;
	vkcv::BufferHandle m_copyPathWorkTileBuffer;
	vkcv::BufferHandle m_fastPathWorkTileBuffer;
};
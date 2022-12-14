
typedef float2 float16_t2;
typedef float3 float16_t3;

int2 FFX_DNSR_Shadows_GetBufferDimensions() {
	return int2(0, 0); // TODO: return buffer dimensions?
}

float2 FFX_DNSR_Shadows_GetInvBufferDimensions() {
	return float2(0, 0); // TODO: return inverse buffer dimensions?
}

float16_t3 FFX_DNSR_Shadows_ReadNormals(int2 p) {
	return float16_t3(0, 0, 0); // TODO: access image or buffer?
}

float16_t2 FFX_DNSR_Shadows_ReadInput(int2 p) {
	return float16_t2(0, 0); // TODO: access image or buffer?
}

float FFX_DNSR_Shadows_ReadDepth(int2 p) {
	return 0; // TODO: access image or buffer?
}

float4x4 FFX_DNSR_Shadows_GetProjectionInverse() {
	return float4x4(0); // TODO: return projection inverse?
}

float FFX_DNSR_Shadows_GetDepthSimilaritySigma() {
	return 0; // TODO: use proper sigma?
}

bool FFX_DNSR_Shadows_IsShadowReciever(uint2 did) {
	return false; // TODO: access image or buffer?
}

uint FFX_DNSR_Shadows_ReadTileMetaData(uint id) {
	return 0; // TODO: access image or buffer?
}

#include "ffx_denoiser_shadows_filter.h"

[numthreads(8, 8, 1)]
void main(int2 group_thread_id : SV_GroupThreadID,
		  uint group_index     : SV_GroupIndex,
		  uint group_id        : SV_GroupID) {
	// TODO: remapping ids to optimization?

	uint2 did = uint2(0, 0);
	bool bWriteResults = false;
	const uint pass = 0;
	const uint stepsize = 0;
	
	FFX_DNSR_Shadows_FilterSoftShadowsPass(
			uint2(group_index, group_id),
			uint2(group_thread_id),
			did,
			bWriteResults,
			pass,
			stepsize
	);
}

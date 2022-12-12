
int2 FFX_DNSR_Shadows_GetBufferDimensions() {
	return int2(0, 0); // TODO: return buffer dimensions?
}

float2 FFX_DNSR_Shadows_GetInvBufferDimensions() {
	return float2(0, 0); // TODO: return inverse buffer dimensions?
}

uint FFX_DNSR_Shadows_ReadRaytracedShadowMask(uint linear_tile_index) {
	return 0; // TODO: access image or buffer?
}

float4x4 FFX_DNSR_Shadows_GetProjectionInverse() {
	return float4x4(0); // TODO: return projection inverse?
}

float4x4 FFX_DNSR_Shadows_GetViewProjectionInverse() {
	return float4x4(0); // TODO: return view-projection inverse?
}

float3 FFX_DNSR_Shadows_ReadNormals(int2 p) {
	return float3(0, 0, 0); // TODO: access image or buffer?
}

float2 FFX_DNSR_Shadows_ReadVelocity(int2 did) {
	return float2(0, 0); // TODO: access image or buffer?
}

float4x4 FFX_DNSR_Shadows_GetReprojectionMatrix() {
	return float4x4(0); // TODO: return reprojection?
}

float4 FFX_DNSR_Shadows_GetEye() {
	return float4(0); // TODO: return eye?
}

float FFX_DNSR_Shadows_ReadPreviousDepth(int2 idx) {
	return 0; // TODO: access image or buffer?
}

#include "ffx_denoiser_shadows_tileclassification.h"

[numthreads(8, 8, 1)]
void main(int2 group_thread_id : SV_GroupThreadID,
		  uint group_index     : SV_GroupIndex,
		  uint group_id        : SV_GroupID) {
	// TODO: remapping ids to optimization?

	FFX_DNSR_Shadows_TileClassification(group_thread_id, group_id);
}

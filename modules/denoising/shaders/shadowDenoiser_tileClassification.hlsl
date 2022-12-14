
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

void FFX_DNSR_Shadows_WriteMetadata(uint id, uint mask) {
	// TODO: access image or buffer?
}

void FFX_DNSR_Shadows_WriteReprojectionResults(uint2 did, float2 results) {
	// TODO: access image or buffer? (results = mean, variance)
}

void FFX_DNSR_Shadows_WriteMoments(uint2 did, float3 moments) {
	// TODO: access image or buffer? (results = mean, variance, temporal sample count)
}

bool FFX_DNSR_Shadows_IsShadowReciever(uint2 did) {
	return false; // TODO: access image or buffer?
}

float FFX_DNSR_Shadows_ReadDepth(uint2 did) {
	return 0.0f; // TODO: access image or buffer?
}

float3 FFX_DNSR_Shadows_ReadPreviousMomentsBuffer(int2 pos) {
	return float3(0.0f); // TODO: access image or buffer?
}

float FFX_DNSR_Shadows_ReadHistory(float2 uv) {
	return 0.0f; // TODO: access image or buffer?
}

bool FFX_DNSR_Shadows_IsFirstFrame() {
	return false; // TODO: do not guess?
}

#include "ffx_denoiser_shadows_tileclassification.h"

[numthreads(8, 8, 1)]
void main(int2 group_thread_id : SV_GroupThreadID,
		  uint group_index     : SV_GroupIndex,
		  uint group_id        : SV_GroupID) {
	// TODO: remapping ids to optimization?

	FFX_DNSR_Shadows_TileClassification(group_index, uint2(group_id, 0));
}

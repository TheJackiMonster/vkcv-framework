
void FFX_DNSR_Reflections_LoadNeighborhood(
		int2 pixel_coordinate,
		out min16float3 radiance,
		out min16float variance,
		out min16float3 normal,
		out float depth,
		int2 screen_size) {
	// TODO: access image or buffer?
}

min16float FFX_DNSR_Reflections_LoadRoughness(int2 id) {
	return min16float(0.0f); // TODO: access image or buffer?
}

bool FFX_DNSR_Reflections_IsGlossyReflection(min16float roughness) {
	return false; // TODO: access image or buffer?
}

bool FFX_DNSR_Reflections_IsMirrorReflection(min16float roughness) {
	return false; // TODO: access image or buffer?
}

min16float FFX_DNSR_Reflections_SampleAverageRadiance(float2 uv8) {
	return min16float(0.0f); // TODO: access image or buffer?
}

void FFX_DNSR_Reflections_StorePrefilteredReflections(
		int2 id,
		min16float3 radiance,
		min16float variance) {
	// TODO: access image or buffer?
}

#include "ffx_denoiser_reflections_prefilter.h"

[numthreads(8, 8, 1)]
void main(int2 group_thread_id : SV_GroupThreadID,
		  uint group_index     : SV_GroupIndex,
		  uint group_id        : SV_GroupID) {
	
}


uint2 FFX_DNSR_Shadows_GetBufferDimensions() {
	return uint2(0, 0); // TODO: return buffer dimensions?
}

bool FFX_DNSR_Shadows_HitsLight(uint2 did, uint2 gtid, uint2 gid) {
	return false; // TODO: access image or buffer?
}

void FFX_DNSR_Shadows_WriteMask(uint linear_tile_index, uint active_bits) {
	// TODO: write to buffer?
}

#include "ffx_denoiser_shadows_prepare.h"

[numthreads(8, 8, 1)]
void main(int2 group_thread_id : SV_GroupThreadID,
		  uint group_index     : SV_GroupIndex,
		  uint group_id        : SV_GroupID) {
	// TODO: remapping ids to optimization?
	
	FFX_DNSR_Shadows_PrepareShadowMask(group_thread_id, group_id);
}

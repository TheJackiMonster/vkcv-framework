
#include "ffx_denoiser_reflections_resolve_temporal.h"

[numthreads(8, 8, 1)]
void main(int2 group_thread_id : SV_GroupThreadID,
		  uint group_index     : SV_GroupIndex,
		  uint group_id        : SV_GroupID) {

}

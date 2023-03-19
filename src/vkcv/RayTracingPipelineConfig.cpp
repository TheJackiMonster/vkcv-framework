
#include "vkcv/RayTracingPipelineConfig.hpp"

namespace vkcv {
	
	RayTracingPipelineConfig::RayTracingPipelineConfig() :
		PipelineConfig() {}
	
	RayTracingPipelineConfig::RayTracingPipelineConfig(
			const ShaderProgram &program,
			const Vector<DescriptorSetLayoutHandle> &layouts) :
			PipelineConfig(program, layouts) {}
	
}

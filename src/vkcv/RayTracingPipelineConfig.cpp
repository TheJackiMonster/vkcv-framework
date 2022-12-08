
#include "vkcv/RayTracingPipelineConfig.hpp"

namespace vkcv {
	
	RayTracingPipelineConfig::RayTracingPipelineConfig() :
		PipelineConfig() {}
	
	RayTracingPipelineConfig::RayTracingPipelineConfig(
			const ShaderProgram &program,
			const std::vector<DescriptorSetLayoutHandle> &layouts) :
			PipelineConfig(program, layouts) {}
	
}

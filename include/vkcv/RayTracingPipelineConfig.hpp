#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/RayTracingPipelineConfig.hpp
 * @brief Ray tracing pipeline config struct to hand over required information to pipeline creation.
 */

#include "Container.hpp"
#include "PipelineConfig.hpp"

namespace vkcv {
	
	/**
	 * @brief Class to configure a ray tracing pipeline before its creation.
	 */
	class RayTracingPipelineConfig : public PipelineConfig {
	public:
		RayTracingPipelineConfig();
		
		RayTracingPipelineConfig(const ShaderProgram &program,
								 const Vector<DescriptorSetLayoutHandle> &layouts);
		
		RayTracingPipelineConfig(const RayTracingPipelineConfig &other) = default;
		RayTracingPipelineConfig(RayTracingPipelineConfig &&other) = default;
		
		~RayTracingPipelineConfig() = default;
		
		RayTracingPipelineConfig &operator=(const RayTracingPipelineConfig &other) = default;
		RayTracingPipelineConfig &operator=(RayTracingPipelineConfig &&other) = default;
	};

}

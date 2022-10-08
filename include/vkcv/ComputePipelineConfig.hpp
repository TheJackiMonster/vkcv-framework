#pragma once
/**
 * @authors Mark Mints, Tobias Frisch
 * @file vkcv/ComputePipelineConfig.hpp
 * @brief Compute pipeline config struct to hand over required information to pipeline creation.
 */

#include "PipelineConfig.hpp"

namespace vkcv {

	/**
	 * @brief Class to configure a compute pipeline before its creation.
	 */
	class ComputePipelineConfig : public PipelineConfig {
		using PipelineConfig::PipelineConfig;
	};

} // namespace vkcv
#pragma once

#include <vkcv/Core.hpp>
#include <vkcv/Handles.hpp>

namespace vkcv::effects {
	
	/**
     * @defgroup vkcv_effects Effects Module
     * A module to apply certain post-processing effects to an image in realtime.
     * @{
     */
	
	class Effect {
	protected:
		Core& m_core;
	
	public:
		explicit Effect(Core& core);
		
		~Effect() = default;
		
		virtual void recordEffect(const CommandStreamHandle& cmdStream,
								  const ImageHandle& input,
								  const ImageHandle& output) = 0;
		
	};
	
	/** @} */
	
}

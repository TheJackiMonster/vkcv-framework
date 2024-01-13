#pragma once

#include <filesystem>

#include <vkcv/ShaderStage.hpp>
#include "ShadyCompiler.hpp"

namespace vkcv::shader {
	
	/**
	 * @addtogroup vkcv_shader
	 * @{
	 */
	
	enum class SlimCompileTarget {
		UNKNOWN
	};
	
	/**
	 * A class to handle Slim runtime shader compilation.
	 */
	class SlimCompiler : public ShadyCompiler {
	private:
		SlimCompileTarget m_target;
	
	public:
		/**
		 * The constructor of a runtime Slim shader compiler instance.
		 *
		 * @param[in] target Compile target (optional)
		 */
		explicit SlimCompiler(SlimCompileTarget target = SlimCompileTarget::UNKNOWN);
		
		/**
		 * Compile a Slim shader from source for a target stage with a custom shader
		 * include path and an event function called if the compilation completes.
		 *
		 * @param[in] shaderStage Shader pipeline stage
		 * @param[in] shaderSource Source of shader
		 * @param[in] compiled Shader compilation event
		 * @param[in] includePath Include path for shaders
		 * @return Result if the compilation succeeds
		 */
		bool compileSource(ShaderStage shaderStage,
						   const std::string& shaderSource,
						   const ShaderCompiledFunction& compiled,
						   const std::filesystem::path& includePath = "") override;
		
	};
	
	/** @} */
	
}

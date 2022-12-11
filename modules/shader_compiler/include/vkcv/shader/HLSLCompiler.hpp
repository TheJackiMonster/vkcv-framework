#pragma once

#include <filesystem>

#include <vkcv/ShaderStage.hpp>
#include "GlslangCompiler.hpp"

namespace vkcv::shader {
	
	/**
	 * @addtogroup vkcv_shader
	 * @{
	 */
	
	enum class HLSLCompileTarget {
		UNKNOWN
	};
	
	/**
	 * A class to handle HLSL runtime shader compilation.
	 */
	class HLSLCompiler : public GlslangCompiler {
	private:
		HLSLCompileTarget m_target;
	
	public:
		/**
		 * The constructor of a runtime HLSL shader compiler instance.
		 *
		 * @param[in] target Compile target (optional)
		 */
		explicit HLSLCompiler(HLSLCompileTarget target = HLSLCompileTarget::UNKNOWN);
		
		/**
		 * Compile a HLSL shader from source for a target stage with a custom shader
		 * include path and an event function called if the compilation completes.
		 *
		 * @param[in] shaderStage Shader pipeline stage
		 * @param[in] shaderSource Source of shader
		 * @param[in] compiled Shader compilation event
		 * @param[in] includePath Include path for shaders
		 * @return Result if the compilation succeeds
		 */
		bool compileSource(ShaderStage shaderStage,
						   const char* shaderSource,
						   const ShaderCompiledFunction& compiled,
						   const std::filesystem::path& includePath = "") override;
		
	};
	
	/** @} */
	
}

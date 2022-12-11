#pragma once

#include <filesystem>

#include <vkcv/ShaderStage.hpp>
#include "GlslangCompiler.hpp"

namespace vkcv::shader {

    /**
     * @addtogroup vkcv_shader
     * @{
     */
     
	enum class GLSLCompileTarget {
		SUBGROUP_OP,
		RAY_TRACING,
		
		UNKNOWN
	};

    /**
     * A class to handle GLSL runtime shader compilation.
     */
	class GLSLCompiler : public GlslangCompiler {
	private:
		GLSLCompileTarget m_target;
		
	public:
        /**
         * The constructor of a runtime GLSL shader compiler instance.
         *
         * @param[in] target Compile target (optional)
         */
		explicit GLSLCompiler(GLSLCompileTarget target = GLSLCompileTarget::UNKNOWN);

        /**
         * Compile a GLSL shader from source for a target stage with a custom shader
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

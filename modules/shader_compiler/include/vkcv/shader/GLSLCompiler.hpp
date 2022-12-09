#pragma once

#include <filesystem>

#include <vkcv/ShaderStage.hpp>
#include "Compiler.hpp"

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
	class GLSLCompiler : public Compiler {
	private:
		GLSLCompileTarget m_target;
		
	public:
        /**
         * The constructor of a runtime GLSL shader compiler instance.
         *
         * @param[in] target Compile target (optional)
         */
		GLSLCompiler(GLSLCompileTarget target = GLSLCompileTarget::UNKNOWN);

        /**
         * The copy-constructor of a runtime GLSL shader compiler instance.
         * @param[in] other Other instance of a GLSL shader compiler instance
         */
		GLSLCompiler(const GLSLCompiler& other);

        /**
         * The move-constructor of a runtime GLSL shader compiler instance.
         * @param[out] other Other instance of a GLSL shader compiler instance
         */
		GLSLCompiler(GLSLCompiler&& other) = default;

        /**
         * The destructor of a runtime GLSL shader compiler instance.
         */
		~GLSLCompiler();

        /**
         * The copy-operator of a runtime GLSL shader compiler instance.
         * @param[in] other Other instance of a GLSL shader compiler instance
         * @return Reference to this instance
         */
		GLSLCompiler& operator=(const GLSLCompiler& other);

        /**
         * The copy-operator of a runtime GLSL shader compiler instance.
         * @param[out] other Other instance of a GLSL shader compiler instance
         * @return Reference to this instance
         */
        GLSLCompiler& operator=(GLSLCompiler&& other) = default;

        /**
         * Compile a GLSL shader from source for a target stage with a custom shader
         * include path and an event function called if the compilation completes.
         * @param[in] shaderStage Shader pipeline stage
         * @param[in] shaderSource Source of shader
         * @param[in] compiled Shader compilation event
         * @param[in] includePath Include path for shaders
         * @return Result if the compilation succeeds
         */
		bool compileSource(ShaderStage shaderStage, const char* shaderSource,
						   const ShaderCompiledFunction& compiled,
						   const std::filesystem::path& includePath = "") override;

        /**
         * Compile a GLSL shader from a specific file path for a target stage with
         * a custom shader include path and an event function called if the
         * compilation completes.
         * @param[in] shaderStage Shader pipeline stage
         * @param[in] shaderPath Filepath of shader
         * @param[in] compiled Shader compilation event
         * @param[in] includePath Include path for shaders
         * @param[in] update Flag to update shaders during runtime
         */
		void compile(ShaderStage shaderStage, const std::filesystem::path& shaderPath,
					 const ShaderCompiledFunction& compiled,
					 const std::filesystem::path& includePath = "", bool update = false) override;
		
	};

    /** @} */
	
}

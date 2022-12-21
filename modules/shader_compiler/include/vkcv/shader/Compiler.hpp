#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include <vkcv/Event.hpp>
#include <vkcv/ShaderStage.hpp>
#include <vkcv/ShaderProgram.hpp>

namespace vkcv::shader {

    /**
     * @defgroup vkcv_shader Shader Compiler Module
     * A module to use runtime shader compilation.
     * @{
     */

    /**
     * An event function type to be called on compilation completion.
     */
	typedef typename event_function<ShaderStage, const std::filesystem::path&>::type ShaderCompiledFunction;
	
	/**
     * An event function type to be called on program compilation completion.
     */
	typedef typename event_function<ShaderProgram&>::type ShaderProgramCompiledFunction;
	
	/**
     * An abstract class to handle runtime shader compilation.
     */
	class Compiler {
	private:
	protected:
        /**
         * A map containing macros for shader compilation.
         */
		std::unordered_map<std::string, std::string> m_defines;
		
		/**
		 * Process the shader source code for further compilation or inclusion.
		 *
		 * Ideally this method will not do anything at all!
		 *
		 * @param[in] shaderSource Source of shader
		 * @return Processed source of shader
		 */
		virtual std::string processShaderSource(const std::string& shaderSource);
		
	public:
        /**
         * Compile a shader from source for a target stage with a custom shader
         * include path and an event function called if the compilation completes.
         *
         * @param[in] shaderStage Shader pipeline stage
         * @param[in] shaderSource Source of shader
         * @param[in] compiled Shader compilation event
         * @param[in] includePath Include path for shaders
         * @return Result if the compilation succeeds
         */
		virtual bool compileSource(ShaderStage shaderStage,
								   const std::string& shaderSource,
								   const ShaderCompiledFunction& compiled,
								   const std::filesystem::path& includePath) = 0;
		
		/**
		 * Compile a shader from source for a target stage with some included headers
		 * as source as well and an event function called if the compilation completes.
		 *
		 * The included shaders will be stored temporarily for shader compilation in
		 * a directory which gets used as include path. So these files can be used via
		 * include instructions in the shader source code as if they were stored in the
		 * same directory.
		 *
		 * @param[in] shaderStage Shader pipeline stage
		 * @param[in] shaderSource Source of shader
		 * @param[in] shaderHeaders Headers of shader
		 * @param[in] compiled Shader compilation event
		 * @return Result if the compilation succeeds
		 */
		bool compileSourceWithHeaders(ShaderStage shaderStage,
									  const std::string& shaderSource,
									  const std::unordered_map<std::filesystem::path, std::string>& shaderHeaders,
									  const ShaderCompiledFunction& compiled);

        /**
         * Compile a shader from a specific file path for a target stage with
         * a custom shader include path and an event function called if the
         * compilation completes.
         *
         * @param[in] shaderStage Shader pipeline stage
         * @param[in] shaderPath Filepath of shader
         * @param[in] compiled Shader compilation event
         * @param[in] includePath Include path for shaders
         * @param[in] update Flag to update shaders during runtime
         */
		virtual void compile(ShaderStage shaderStage,
							 const std::filesystem::path& shaderPath,
							 const ShaderCompiledFunction& compiled,
							 const std::filesystem::path& includePath,
							 bool update) = 0;
		
		/**
         * Compile a shader program from a specific map of given file paths for
         * target pipeline stages with a custom shader include path and an event
         * function called if the compilation completes.
         *
         * @param[in,out] program Shader program
         * @param[in] stages Shader pipeline stages
         * @param[in] compiled Shader program compilation event
         * @param[in] includePath Include path for shaders
         * @param[in] update Flag to update shaders during runtime
         */
		void compileProgram(ShaderProgram& program,
							const std::unordered_map<ShaderStage, const std::filesystem::path>& stages,
							const ShaderProgramCompiledFunction& compiled,
							const std::filesystem::path& includePath = "",
							bool update = false);
		
		/**
         * Return the definition value of a macro for shader compilation.
         *
         * @param[in] name Macro definition name
         * @return Macro definition value
         */
		[[nodiscard]]
		std::string getDefine(const std::string& name) const;

        /**
         * Set a macro for shader compilation.
         *
         * @param[in] name Macro definition name
         * @param[in] value Macro definition value
         */
		void setDefine(const std::string& name, const std::string& value);
		
	};

    /** @} */
	
}

#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include <vkcv/Event.hpp>
#include <vkcv/ShaderStage.hpp>

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
     * An abstract class to handle runtime shader compilation.
     */
	class Compiler {
	private:
	protected:
        /**
         * A map containing macros for shader compilation.
         */
		std::unordered_map<std::string, std::string> m_defines;
		
	public:
        /**
         * Compile a shader from source for a target stage with a custom shader
         * include path and an event function called if the compilation completes.
         * @param[in] shaderStage Shader pipeline stage
         * @param[in] shaderSource Source of shader
         * @param[in] compiled Shader compilation event
         * @param[in] includePath Include path for shaders
         * @return Result if the compilation succeeds
         */
		virtual bool compileSource(ShaderStage shaderStage, const char* shaderSource,
								   const ShaderCompiledFunction& compiled,
								   const std::filesystem::path& includePath) = 0;

        /**
         * Compile a shader from a specific file path for a target stage with
         * a custom shader include path and an event function called if the
         * compilation completes.
         * @param[in] shaderStage Shader pipeline stage
         * @param[in] shaderPath Filepath of shader
         * @param[in] compiled Shader compilation event
         * @param[in] includePath Include path for shaders
         * @param[in] update Flag to update shaders during runtime
         */
		virtual void compile(ShaderStage shaderStage, const std::filesystem::path& shaderPath,
							 const ShaderCompiledFunction& compiled,
							 const std::filesystem::path& includePath, bool update) = 0;

        /**
         * Return the definition value of a macro for shader compilation.
         * @param[in] name Macro definition name
         * @return Macro definition value
         */
		std::string getDefine(const std::string& name) const;

        /**
         * Set a macro for shader compilation.
         * @param[in] name Macro definition name
         * @param[in] value Macro definition value
         */
		void setDefine(const std::string& name, const std::string& value);
	};

    /** @} */
	
}

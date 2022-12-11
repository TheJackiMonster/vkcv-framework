#pragma once

#include <filesystem>

#include <vkcv/ShaderStage.hpp>
#include "Compiler.hpp"

namespace vkcv::shader {
	
	/**
     * @addtogroup vkcv_shader
     * @{
     */
	
	/**
	 * An abstract class to handle Glslang runtime shader compilation.
	 */
	class GlslangCompiler : public Compiler {
	protected:
		/**
		 *
		 *
		 * @param[in] shaderPath Filepath of shader
		 * @param[out] spirv
		 * @return
		 */
		static bool writeSpirvCode(const std::filesystem::path &shaderPath,
								   const std::vector<uint32_t>& spirv);
		
		/**
		 *
		 *
		 * @param[in] shaderPath Filepath of shader
		 * @return
		 */
		static std::vector<char> readShaderCode(const std::filesystem::path &shaderPath);
		
	public:
		/**
		 * The constructor of a runtime Glslang shader compiler instance.
		 *
		 * @param[in] target Compile target (optional)
		 */
		GlslangCompiler();
		
		/**
		 * The copy-constructor of a runtime Glslang shader compiler instance.
		 *
		 * @param[in] other Other instance of a Glslang shader compiler instance
		 */
		GlslangCompiler(const GlslangCompiler& other);
		
		/**
		 * The move-constructor of a runtime Glslang shader compiler instance.
		 *
		 * @param[out] other Other instance of a Glslang shader compiler instance
		 */
		GlslangCompiler(GlslangCompiler&& other) = default;
		
		/**
		 * The destructor of a runtime Glslang shader compiler instance.
		 */
		~GlslangCompiler();
		
		/**
		 * The copy-operator of a runtime Glslang shader compiler instance.
		 *
		 * @param[in] other Other instance of a Glslang shader compiler instance
		 * @return Reference to this instance
		 */
		GlslangCompiler& operator=(const GlslangCompiler& other);
		
		/**
		 * The copy-operator of a runtime Glslang shader compiler instance.
		 *
		 * @param[out] other Other instance of a Glslang shader compiler instance
		 * @return Reference to this instance
		 */
		GlslangCompiler& operator=(GlslangCompiler&& other) = default;
		
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
		void compile(ShaderStage shaderStage,
					 const std::filesystem::path& shaderPath,
					 const ShaderCompiledFunction& compiled,
					 const std::filesystem::path& includePath = "",
					 bool update = false) override;
		
	};
	
	/** @} */

}

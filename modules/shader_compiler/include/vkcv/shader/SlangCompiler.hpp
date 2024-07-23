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
	 * An abstract class to handle Slang runtime shader compilation.
	 */
	class SlangCompiler : public Compiler {
	public:
		/**
		 * The constructor of a runtime Slang shader compiler instance.
		 */
		SlangCompiler();
		
		/**
		 * The copy-constructor of a runtime Slang shader compiler instance.
		 *
		 * @param[in] other Other instance of a Slang shader compiler instance
		 */
		SlangCompiler(const SlangCompiler& other);
		
		/**
		 * The move-constructor of a runtime Slang shader compiler instance.
		 *
		 * @param[out] other Other instance of a Slang shader compiler instance
		 */
		SlangCompiler(SlangCompiler&& other) = default;
		
		/**
		 * The destructor of a runtime Slang shader compiler instance.
		 */
		~SlangCompiler();
		
		/**
		 * The copy-operator of a runtime Slang shader compiler instance.
		 *
		 * @param[in] other Other instance of a Slang shader compiler instance
		 * @return Reference to this instance
		 */
		SlangCompiler& operator=(const SlangCompiler& other);
		
		/**
		 * The copy-operator of a runtime Slang shader compiler instance.
		 *
		 * @param[out] other Other instance of a Slang shader compiler instance
		 * @return Reference to this instance
		 */
		SlangCompiler& operator=(SlangCompiler&& other) = default;
		
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

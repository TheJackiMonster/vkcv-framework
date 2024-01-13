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
	 * An abstract class to handle Shady runtime shader compilation.
	 */
    class ShadyCompiler : public Compiler {
    public:
        /**
		 * The constructor of a runtime Shady shader compiler instance.
		 */
        ShadyCompiler();

        /**
		 * The copy-constructor of a runtime Shady shader compiler instance.
		 *
		 * @param[in] other Other instance of a Shady shader compiler instance
		 */
        ShadyCompiler(const ShadyCompiler& other) = default;

        /**
		 * The move-constructor of a runtime Shady shader compiler instance.
		 *
		 * @param[out] other Other instance of a Shady shader compiler instance
		 */
        ShadyCompiler(ShadyCompiler&& other) = default;

        /**
		 * The destructor of a runtime Shady shader compiler instance.
		 */
        ~ShadyCompiler() = default;

        /**
		 * The copy-operator of a runtime Shady shader compiler instance.
		 *
		 * @param[in] other Other instance of a Shady shader compiler instance
		 * @return Reference to this instance
		 */
        ShadyCompiler& operator=(const ShadyCompiler& other) = default;

        /**
		 * The copy-operator of a runtime Shady shader compiler instance.
		 *
		 * @param[out] other Other instance of a Shady shader compiler instance
		 * @return Reference to this instance
		 */
        ShadyCompiler& operator=(ShadyCompiler&& other) = default;

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

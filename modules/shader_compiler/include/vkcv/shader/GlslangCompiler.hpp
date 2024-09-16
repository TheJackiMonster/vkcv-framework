#pragma once

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
	public:
		/**
		 * The constructor of a runtime Glslang shader compiler instance.
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
		
	};
	
	/** @} */

}
